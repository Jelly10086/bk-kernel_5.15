from pathlib import Path
import re
import subprocess
import tempfile
import unittest

BUILD = Path(__file__).resolve().parents[1]


class ControllerTests(unittest.TestCase):
    def test_actual_sampler_requests_and_cluster_votes(self):
        source = (BUILD / 'modules/bkk-perf/bkk_perf.c').read_text()
        structure = re.search(r'^struct bkk_cluster \{.*?^\};', source, re.M | re.S).group()
        constants = '\n'.join(re.findall(r'^#define BKK_.*$', source, re.M))
        names = ['bkk_release_vote', 'bkk_release_votes', 'bkk_stop', 'bkk_sample',
                 'bkk_vote', 'bkk_tick', 'request_store']
        functions = '\n\n'.join(re.search(
            r'^static [^\n]*\b' + name + r'\([^;]*?\n\{.*?^\}', source,
            re.M | re.S).group() for name in names)
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include "policy.h"
typedef uint64_t u64;
#define HZ 100
CONSTANTS
#define FREQ_QOS_MIN 1
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define READ_ONCE(a) (a)
#define time_after_eq(a,b) ((long)((a) - (b)) >= 0)
#define for_each_cpu(cpu,mask) for ((cpu)=0; (cpu)<8; (cpu)++) if (*(mask) & (1U << (cpu)))
#define cpufreq_for_each_valid_entry(e,t) for ((e)=(t); (e)->frequency; (e)++)
#define system_freezable_power_efficient_wq NULL
struct cpufreq_frequency_table { unsigned int frequency; };
struct freq_qos_request { bool active; unsigned int value; };
struct cpufreq_policy {
    unsigned int *related_cpus, max;
    struct { unsigned int min_freq, max_freq; } cpuinfo;
    struct cpufreq_frequency_table *freq_table;
    int constraints;
};
struct work_struct { int unused; };
struct delayed_work { int unused; };
struct kobject { int unused; };
struct kobj_attribute { int unused; };
STRUCTURE
static struct bkk_cluster clusters[3];
static struct cpufreq_policy policies[3];
static unsigned int masks[3] = {7, 120, 128};
static struct cpufreq_frequency_table table[] = {
    {300000}, {600000}, {1200000}, {1800000}, {2100000}, {2400000}, {3000000}, {0}
};
static unsigned int mode, little;
static unsigned long jiffies, deadline, queued_delay;
static bool suspended, hot;
static int battery_temp, soc_temp, bkk_lock, fail_policy;
static const char *reason;
static struct delayed_work bkk_work;
static u64 prev_idle[8], prev_wall[8], idle_time[8], wall_time[8], io_time[8];
static bool online[8];
static void mutex_lock(int *lock) { (void)lock; }
static void mutex_unlock(int *lock) { (void)lock; }
static bool cpu_online(unsigned int cpu) { return online[cpu]; }
static u64 div64_u64(u64 a, u64 b) { return a / b; }
static u64 get_cpu_idle_time_us(unsigned int cpu, u64 *wall) {
    *wall = wall_time[cpu]; return idle_time[cpu];
}
static u64 get_cpu_iowait_time_us(unsigned int cpu, void *unused) { return io_time[cpu]; }
static bool freq_qos_request_active(struct freq_qos_request *r) { return r->active; }
static int freq_qos_remove_request(struct freq_qos_request *r) { r->active=false; return 0; }
static int freq_qos_update_request(struct freq_qos_request *r, unsigned int value) {
    r->value=value; return 0;
}
static int freq_qos_add_request(int *constraint, struct freq_qos_request *r,
                                int type, unsigned int value) {
    assert(type == FREQ_QOS_MIN);
    if (*constraint == fail_policy) return -EIO;
    r->active=true; r->value=value; return 0;
}
static void cpufreq_cpu_put(struct cpufreq_policy *p) { (void)p; }
static void cancel_delayed_work(struct delayed_work *work) { queued_delay=0; }
static bool queue_delayed_work(void *queue, struct delayed_work *work, unsigned long delay) {
    queued_delay=delay; return true;
}
static bool sysfs_streq(const char *a, const char *b) { return !strcmp(a,b); }
static int bkk_get_policies(void) {
    for (int i=0; i<3; i++) clusters[i].policy=&policies[i];
    memset(prev_idle,0,sizeof(prev_idle)); memset(prev_wall,0,sizeof(prev_wall));
    return 0;
}
FUNCTIONS
static void reset(void) {
    memset(clusters,0,sizeof(clusters));
    for (int i=0; i<3; i++) {
        policies[i]=(struct cpufreq_policy){.related_cpus=&masks[i], .max=3000000,
            .cpuinfo={300000,3000000}, .freq_table=table, .constraints=i};
    }
    for (int cpu=0; cpu<8; cpu++) {
        online[cpu]=true; idle_time[cpu]=io_time[cpu]=0; wall_time[cpu]=1000000;
    }
    jiffies=deadline=queued_delay=mode=little=0;
    hot=suspended=false; fail_policy=-1;
}
static ssize_t request(const char *text) {
    return request_store(NULL,NULL,text,strlen(text));
}
static void frame(unsigned int big, unsigned int prime) {
    for (int cpu=0; cpu<8; cpu++) {
        unsigned int busy=cpu == 3 ? big : cpu == 7 ? prime : 0;
        wall_time[cpu]+=1000000;
        idle_time[cpu]+=1000000ULL*(100-busy)/100;
    }
    jiffies+=HZ;
    bkk_tick(NULL);
}
int main(void) {
    reset();
    assert(request("manual 40000 70000 1") > 0);
    assert(clusters[1].floor == 2100000 && clusters[2].floor == 2100000);
    assert(!clusters[0].floor && !strcmp(reason,"boost") && queued_delay == HZ);
    assert(request("off") > 0 && !mode && !clusters[1].floor && !clusters[2].floor);

    reset();
    assert(request("auto 40000 70000 1") > 0);
    frame(100,0);
    assert(clusters[1].busy == 25 && clusters[1].peak == 100);
    assert(clusters[1].load.high == 1 && !clusters[1].floor);
    assert(request("auto 40000 70000 1") > 0);
    assert(clusters[1].load.high == 1);
    frame(100,0);
    assert(clusters[1].floor == 1800000 && !clusters[2].floor && !clusters[0].floor);
    frame(0,100);
    assert(clusters[1].floor && !clusters[2].floor);
    frame(0,100);
    assert(clusters[1].floor && clusters[2].floor == 1800000);
    frame(0,100);
    assert(clusters[1].floor);
    frame(0,100);
    assert(!clusters[1].floor && clusters[2].floor);

    reset();
    assert(request("manual 45999 87000 1") > 0 && clusters[1].floor);
    assert(request("manual 46000 87000 1") > 0 && !clusters[1].floor && !clusters[2].floor);
    assert(request("manual 43000 80001 1") > 0 && hot && !clusters[1].floor);
    assert(request("manual 43000 80000 1") > 0 && !hot && clusters[1].floor);
    assert(request("manual 40000 87001 1") > 0 && !clusters[1].floor);
    assert(request("manual 40000 70000 0") > 0 && !mode);

    reset();
    fail_policy=2;
    assert(request("manual 40000 70000 1") == -EIO);
    assert(!mode && !clusters[1].floor && !clusters[1].request.active);
    assert(!clusters[2].floor && !strcmp(reason,"qos_error"));

    reset();
    assert(request("manual 40000 70000 1") > 0);
    policies[1].max=300000;
    assert(!bkk_vote(&clusters[1]) && clusters[1].floor == 300000);
    policies[1].max=1500000;
    assert(!bkk_vote(&clusters[1]) && clusters[1].floor <= policies[1].max);
    jiffies=deadline;
    bkk_tick(NULL);
    assert(!mode && !clusters[1].floor && !clusters[2].floor);
    assert(!strcmp(reason,"lease_expired"));

    reset();
    assert(request("auto 40000 70000 1") > 0);
    online[3]=false;
    frame(100,0);
    assert(!clusters[1].peak);
    online[3]=true;
    frame(100,0);
    assert(!clusters[1].peak);
    frame(100,0);
    assert(clusters[1].peak == 100 && !clusters[1].floor);
    puts("controller scenarios passed");
    return 0;
}
'''.replace('CONSTANTS', constants).replace('STRUCTURE', structure).replace('FUNCTIONS', functions)
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'controller.c').write_text(harness)
            subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-Wno-unused-parameter', '-I', str(BUILD / 'modules/bkk-perf'),
                            str(root / 'controller.c'), '-o', str(root / 'controller')], check=True)
            result = subprocess.run([str(root / 'controller')], check=True, capture_output=True, text=True)
            self.assertEqual(result.stdout.strip(), 'controller scenarios passed')


if __name__ == '__main__':
    unittest.main()
