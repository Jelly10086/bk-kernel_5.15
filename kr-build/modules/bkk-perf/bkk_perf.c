// SPDX-License-Identifier: GPL-2.0-only
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pm.h>
#include <linux/pm_qos.h>
#include <linux/suspend.h>
#include <linux/sysfs.h>
#include <linux/tick.h>
#include <linux/workqueue.h>

#include "policy.h"

#define BKK_CPUS 8
#define BKK_CLUSTERS 3
#define BKK_INTERVAL HZ
#define BKK_LEASE (8 * HZ)

struct bkk_cluster {
	struct cpufreq_policy *policy;
	struct freq_qos_request request;
	unsigned int floor;
	unsigned int busy;
	unsigned int peak;
	struct bkk_load_state load;
};

static DEFINE_MUTEX(bkk_lock);
static struct bkk_cluster clusters[BKK_CLUSTERS];
static struct kobject *bkk_kobj;
static struct delayed_work bkk_work;
static u64 prev_idle[BKK_CPUS];
static u64 prev_wall[BKK_CPUS];
static unsigned long deadline;
static unsigned int mode;
static unsigned int little;
static int battery_temp;
static int soc_temp;
static bool suspended;
static bool hot;
static const char *reason = "disabled";

static void bkk_release_vote(struct bkk_cluster *c)
{
	if (freq_qos_request_active(&c->request))
		freq_qos_remove_request(&c->request);
	c->floor = 0;
}

static void bkk_release_votes(void)
{
	unsigned int i;

	for (i = 0; i < BKK_CLUSTERS; i++) {
		bkk_release_vote(&clusters[i]);
		memset(&clusters[i].load, 0, sizeof(clusters[i].load));
	}
}

static void bkk_stop(const char *why)
{
	unsigned int i;

	mode = 0;
	deadline = 0;
	cancel_delayed_work(&bkk_work);
	bkk_release_votes();
	for (i = 0; i < BKK_CLUSTERS; i++) {
		if (clusters[i].policy)
			cpufreq_cpu_put(clusters[i].policy);
		clusters[i].policy = NULL;
		clusters[i].busy = 0;
		clusters[i].peak = 0;
	}
	reason = why;
}

static int bkk_get_policies(void)
{
	struct cpufreq_policy *p;
	unsigned int cpu, i, count = 0, weights = 0;

	if (nr_cpu_ids != BKK_CPUS)
		return -ENODEV;
	for (cpu = 0; cpu < BKK_CPUS; cpu++) {
		p = cpufreq_cpu_get(cpu);
		if (!p)
			continue;
		for (i = 0; i < count; i++)
			if (clusters[i].policy == p)
				break;
		if (i != count) {
			cpufreq_cpu_put(p);
			continue;
		}
		if (count == BKK_CLUSTERS || !p->freq_table) {
			cpufreq_cpu_put(p);
			goto unsupported;
		}
		clusters[count++].policy = p;
		weights |= BIT(cpumask_weight(p->related_cpus));
	}
	if (count != BKK_CLUSTERS || weights != (BIT(1) | BIT(3) | BIT(4)))
		goto unsupported;
	little = 0;
	for (i = 1; i < BKK_CLUSTERS; i++)
		if (clusters[i].policy->cpuinfo.max_freq <
		    clusters[little].policy->cpuinfo.max_freq)
			little = i;
	if (cpumask_weight(clusters[little].policy->related_cpus) != 3)
		goto unsupported;
	memset(prev_idle, 0, sizeof(prev_idle));
	memset(prev_wall, 0, sizeof(prev_wall));
	return 0;

unsupported:
	bkk_stop("unsupported_cpu");
	return -ENODEV;
}

static void bkk_sample(struct bkk_cluster *c)
{
	u64 idle, iowait, wall, dt, di, busy = 0, total = 0;
	unsigned int cpu, peak = 0;

	for_each_cpu(cpu, c->policy->related_cpus) {
		if (!cpu_online(cpu)) {
			prev_wall[cpu] = 0;
			continue;
		}
		idle = get_cpu_idle_time_us(cpu, &wall);
		iowait = get_cpu_iowait_time_us(cpu, NULL);
		if (idle == -1ULL || iowait == -1ULL) {
			prev_wall[cpu] = 0;
			continue;
		}
		idle += iowait;
		if (prev_wall[cpu] && wall > prev_wall[cpu] &&
		    idle >= prev_idle[cpu]) {
			dt = wall - prev_wall[cpu];
			di = min(idle - prev_idle[cpu], dt);
			busy += dt - di;
			total += dt;
			peak = max(peak, (unsigned int)div64_u64((dt - di) * 100, dt));
		}
		prev_wall[cpu] = wall;
		prev_idle[cpu] = idle;
	}
	c->busy = total ? div64_u64(busy * 100, total) : 0;
	c->peak = peak;
}

static int bkk_vote(struct bkk_cluster *c)
{
	struct cpufreq_frequency_table *entry;
	unsigned int cap, target, floor = 0;
	int ret;

	/* Only add a minimum request; vendor and thermal maximums stay in charge. */
	cap = min(READ_ONCE(c->policy->max), c->policy->cpuinfo.max_freq);
	target = max(c->policy->cpuinfo.min_freq, cap / 100 * (mode == 2 ? 70 : 60));
	target = min(target, cap);
	cpufreq_for_each_valid_entry(entry, c->policy->freq_table)
		if (entry->frequency <= target && entry->frequency > floor)
			floor = entry->frequency;
	if (!floor)
		return -ERANGE;
	if (freq_qos_request_active(&c->request))
		ret = freq_qos_update_request(&c->request, floor);
	else
		ret = freq_qos_add_request(&c->policy->constraints,
					   &c->request, FREQ_QOS_MIN, floor);
	if (ret < 0)
		return ret;
	c->floor = floor;
	return 0;
}

static void bkk_tick(struct work_struct *work)
{
	unsigned int i;
	bool boosting = false;

	mutex_lock(&bkk_lock);
	if (!mode)
		goto out;
	if (suspended || time_after_eq(jiffies, deadline)) {
		bkk_stop(suspended ? "suspend" : "lease_expired");
		goto out;
	}
	for (i = 0; i < BKK_CLUSTERS; i++) {
		bkk_sample(&clusters[i]);
		if (i == little)
			continue;
		if (bkk_decide(&clusters[i].load, clusters[i].peak, mode == 2, hot)) {
			if (bkk_vote(&clusters[i])) {
				bkk_stop("qos_error");
				goto out;
			}
			boosting = true;
		} else {
			bkk_release_vote(&clusters[i]);
		}
	}
	reason = hot ? "thermal" : boosting ? "boost" : "idle";
	queue_delayed_work(system_freezable_power_efficient_wq,
			   &bkk_work, BKK_INTERVAL);
out:
	mutex_unlock(&bkk_lock);
}

static ssize_t request_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t count)
{
	char command[8], extra;
	unsigned int next_mode, screen;
	int battery, soc, ret = 0;
	unsigned int i;

	mutex_lock(&bkk_lock);
	if (sysfs_streq(buf, "off")) {
		bkk_stop("disabled");
		goto out;
	}
	if (sscanf(buf, "%7s %d %d %u %c", command, &battery, &soc,
		   &screen, &extra) != 4 ||
	    (strcmp(command, "auto") && strcmp(command, "manual")) ||
	    battery < 0 || battery > 100000 || soc < 1000 || soc > 150000 ||
	    screen > 1) {
		bkk_stop("invalid_sensor_data");
		ret = -EINVAL;
		goto out;
	}
	battery_temp = battery;
	soc_temp = soc;
	hot = bkk_thermal(hot, battery, soc);
	if (!screen || suspended) {
		bkk_stop(suspended ? "suspend" : "screen_off");
		goto out;
	}
	next_mode = !strcmp(command, "auto") ? 1 : 2;
	if (!mode) {
		ret = bkk_get_policies();
		if (ret)
			goto out;
		for (i = 0; i < BKK_CLUSTERS; i++)
			bkk_sample(&clusters[i]);
	} else if (mode != next_mode) {
		bkk_release_votes();
	}
	mode = next_mode;
	deadline = jiffies + BKK_LEASE;
	if (hot) {
		bkk_release_votes();
		reason = "thermal";
	} else if (mode == 2) {
		for (i = 0; i < BKK_CLUSTERS; i++) {
			if (i == little)
				continue;
			ret = bkk_vote(&clusters[i]);
			if (ret) {
				bkk_stop("qos_error");
				goto out;
			}
			clusters[i].load.boost = true;
		}
		reason = "boost";
	} else {
		reason = "idle";
		for (i = 0; i < BKK_CLUSTERS; i++)
			if (clusters[i].floor)
				reason = "boost";
	}
	/* Refreshing a lease must not reset the automatic sampling interval. */
	queue_delayed_work(system_freezable_power_efficient_wq,
			   &bkk_work, BKK_INTERVAL);
out:
	mutex_unlock(&bkk_lock);
	return ret ? ret : (ssize_t)count;
}

static ssize_t status_show(struct kobject *kobj, struct kobj_attribute *attr,
			   char *buf)
{
	ssize_t count;

	mutex_lock(&bkk_lock);
	count = sysfs_emit(buf,
		"version=%s\nmode=%u\nstate=%s\nbattery_mC=%d\nsoc_mC=%d\n"
		"cluster0_busy=%u\ncluster1_busy=%u\ncluster2_busy=%u\n"
		"cluster0_peak=%u\ncluster1_peak=%u\ncluster2_peak=%u\n"
		"cluster0_floor=%u\ncluster1_floor=%u\ncluster2_floor=%u\n",
		BKK_VERSION, mode, reason, battery_temp, soc_temp,
		clusters[0].busy, clusters[1].busy, clusters[2].busy,
		clusters[0].peak, clusters[1].peak, clusters[2].peak,
		clusters[0].floor, clusters[1].floor, clusters[2].floor);
	mutex_unlock(&bkk_lock);
	return count;
}

static struct kobj_attribute request_attr = __ATTR_WO(request);
static struct kobj_attribute status_attr = __ATTR_RO(status);
static struct attribute *bkk_attrs[] = {
	&request_attr.attr,
	&status_attr.attr,
	NULL,
};
static const struct attribute_group bkk_group = { .attrs = bkk_attrs };

static int bkk_pm_event(struct notifier_block *nb, unsigned long event, void *p)
{
	mutex_lock(&bkk_lock);
	switch (event) {
	case PM_SUSPEND_PREPARE:
	case PM_HIBERNATION_PREPARE:
	case PM_RESTORE_PREPARE:
		suspended = true;
		bkk_stop("suspend");
		break;
	case PM_POST_SUSPEND:
	case PM_POST_HIBERNATION:
	case PM_POST_RESTORE:
		suspended = false;
		break;
	}
	mutex_unlock(&bkk_lock);
	return NOTIFY_OK;
}

static struct notifier_block bkk_pm = { .notifier_call = bkk_pm_event };

static int __init bkk_init(void)
{
	int ret;

	INIT_DELAYED_WORK(&bkk_work, bkk_tick);
	bkk_kobj = kobject_create_and_add("bkk_control", kernel_kobj);
	if (!bkk_kobj)
		return -ENOMEM;
	ret = register_pm_notifier(&bkk_pm);
	if (ret)
		goto put;
	ret = sysfs_create_group(bkk_kobj, &bkk_group);
	if (!ret)
		return 0;
	unregister_pm_notifier(&bkk_pm);
put:
	kobject_put(bkk_kobj);
	return ret;
}

static void __exit bkk_exit(void)
{
	sysfs_remove_group(bkk_kobj, &bkk_group);
	unregister_pm_notifier(&bkk_pm);
	mutex_lock(&bkk_lock);
	bkk_stop("unloaded");
	mutex_unlock(&bkk_lock);
	cancel_delayed_work_sync(&bkk_work);
	kobject_put(bkk_kobj);
}

module_init(bkk_init);
module_exit(bkk_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("RinnRei");
MODULE_DESCRIPTION("Opt-in leased CPU QoS for sheng");
MODULE_VERSION(BKK_VERSION);
