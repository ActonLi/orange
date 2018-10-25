#pragma once
#include "orange.h"
#include "orange_bitops.h"

typedef atomic_uchar orange_atomic8_t;
typedef atomic_int   orange_atomic_t;

#define orange_atomic8_read(v) atomic_load((v))
#define orange_atomic8_set(v, i) atomic_store((v), (i))

#define orange_atomic8_add(v, i) atomic_fetch_add((v), (i))
#define orange_atomic8_inc(v) atomic_fetch_add((v), 1)
#define orange_atomic8_sub(v, i) atomic_fetch_sub((v), (i))
#define orange_atomic8_dec(v) atomic_fetch_sub((v), 1)
#define orange_atomic8_dec_and_test(v) (atomic_fetch_sub((v), 1) == 1)
#define orange_atomic8_test_and_inc(v) (atomic_fetch_add((v), 1) == 0)
#define orange_atomic8_fetch_and_inc(v) atomic_fetch_add((v), 1)

#define orange_atomic_read(v) atomic_load((v))
#define orange_atomic_set(v, i) atomic_store((v), (i))
#define orange_atomic_add(v, i) atomic_fetch_add((v), (i))
#define orange_atomic_inc(v) atomic_fetch_add((v), 1)
#define orange_atomic_sub(v, i) atomic_fetch_sub((v), (i))
#define orange_atomic_dec(v) atomic_fetch_sub((v), 1)
#define orange_atomic_dec_and_test(v) (atomic_fetch_sub((v), 1) == 1)
#define orange_atomic_test_and_inc(v) (atomic_fetch_add((v), 1) == 0)
#define orange_atomic_fetch_and_inc(v) atomic_fetch_add((v), 1)

#define orange_atomic_cmpset(v, exp, src) atomic_compare_exchange_strong((v), &(exp), src)

static __inline__ int orange_atomic_add_unless(orange_atomic_t* v, int a, int u)
{
	int c, ret;
	c = orange_atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;

		ret = orange_atomic_cmpset((v), c, c + (a));
		if (likely(ret != 0)) {
			break;
		}
		c = orange_atomic_read(v);
	}
	return c != (u);
}

#define orange_atomic_inc_not_zero(v) orange_atomic_add_unless((v), 1, 0)

typedef atomic_llong  orange_atomic64_t;
typedef atomic_ullong orange_atomicu64_t;

#define orange_atomic64_read(v) atomic_load((v))
#define orange_atomic64_set(v, i) atomic_store((v), (i))

#define orange_atomic64_add(v, i) atomic_fetch_add((v), (i))
#define orange_atomic64_inc(v) atomic_fetch_add((v), 1)
#define orange_atomic64_sub(v, i) atomic_fetch_sub((v), (i))
#define orange_atomic64_dec(v) atomic_fetch_sub((v), 1)
#define orange_atomic64_dec_and_test(v) (atomic_fetch_sub((v), 1) == 1)
#define orange_atomic64_test_and_inc(v) (atomic_fetch_add((v), 1) == 0)
#define orange_atomic64_fetch_and_inc(v) atomic_fetch_add((v), 1)

#define orange_atomic64_cmpset(v, exp, src) atomic_compare_exchange_strong((v), &(exp), src)

#define orange_atomicu64_read(v) atomic_load((v))
#define orange_atomicu64_set(v, i) atomic_store((v), (i))

#define orange_atomicu64_add(v, i) atomic_fetch_add((v), (i))
#define orange_atomicu64_inc(v) atomic_fetch_add((v), 1)
#define orange_atomicu64_sub(v, i) atomic_fetch_sub((v), (i))
#define orange_atomicu64_dec(v) atomic_fetch_sub((v), 1)
#define orange_atomicu64_dec_and_test(v) (atomic_fetch_sub((v), 1) == 1)
#define orange_atomicu64_test_and_inc(v) (atomic_fetch_add((v), 1) == 0)
#define orange_atomicu64_fetch_and_inc(v) atomic_fetch_add((v), 1)

#define orange_atomicu64_cmpset(v, exp, src) atomic_compare_exchange_strong((v), &(exp), src)

static __inline__ int orange_atomicu64_add_unless(orange_atomicu64_t* v, long a, long u)
{
	long c, ret;
	c = orange_atomicu64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;

		ret = orange_atomicu64_cmpset((v), c, c + (a));
		if (likely(ret == 1))
			break;
		c = orange_atomic64_read(v);
	}
	return c != (u);
}
#define orange_atomicu64_inc_not_zero(v) orange_atomicu64_add_unless((v), 1, 0)
