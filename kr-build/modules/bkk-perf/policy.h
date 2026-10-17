/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef BKK_POLICY_H
#define BKK_POLICY_H

struct bkk_load_state {
	unsigned int high;
	unsigned int low;
	bool boost;
	bool hot;
};

static inline bool bkk_thermal(struct bkk_load_state *s, int battery, int soc)
{
	if (battery >= 46000 || soc > 87000)
		s->hot = true;
	else if (battery <= 43000 && soc <= 80000)
		s->hot = false;
	return s->hot;
}

static inline bool bkk_decide(struct bkk_load_state *s, unsigned int busy,
			      bool manual)
{
	if (s->hot) {
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
		if (s->high < 3)
			s->high++;
		if (s->high == 3)
			s->boost = true;
	} else if (busy <= 45) {
		s->high = 0;
		if (s->low < 6)
			s->low++;
		if (s->low == 6)
			s->boost = false;
	} else {
		s->high = 0;
		s->low = 0;
	}
	return s->boost;
}

#endif
