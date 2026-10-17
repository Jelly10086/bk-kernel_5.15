/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef BKK_POLICY_H
#define BKK_POLICY_H

struct bkk_load_state {
	unsigned int high;
	unsigned int low;
	bool boost;
};

static inline bool bkk_thermal(bool hot, int battery, int soc)
{
	if (battery >= 46000 || soc > 87000)
		return true;
	else if (battery <= 43000 && soc <= 80000)
		return false;
	return hot;
}

static inline bool bkk_decide(struct bkk_load_state *s, unsigned int busy,
			      bool manual, bool hot)
{
	if (hot) {
		s->high = 0;
		s->low = 0;
		s->boost = false;
		return false;
	}
	if (manual) {
		s->boost = true;
		return true;
	}
	if (busy >= 75) {
		s->low = 0;
		if (s->high < 2)
			s->high++;
		if (s->high == 2)
			s->boost = true;
	} else if (busy <= 45) {
		s->high = 0;
		if (s->low < 4)
			s->low++;
		if (s->low == 4)
			s->boost = false;
	} else {
		s->high = 0;
		s->low = 0;
	}
	return s->boost;
}

#endif
