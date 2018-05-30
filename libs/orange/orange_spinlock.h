#ifndef __ORANGE_SPINLOCK_H__
#define __ORANGE_SPINLOCK_H__

#include "orange_log.h"

#ifdef _KERNEL

typedef struct mtx orange_spinlock_t;
#define orange_spinlock_init_ex(lock, name, type) mtx_init(lock, name, NULL, type)
#define orange_spinlock_init(lock, name) mtx_init(lock, name, NULL, MTX_DEF)
#define orange_spinlock_destroy(lock) mtx_destroy(lock)
#define orange_spinlock_lock(lock) mtx_lock(lock)
#define orange_spinlock_unlock(lock) mtx_unlock(lock)
#define orange_spinlock_trylock(lock) mtx_trylock(lock)

#else

#ifndef ORANGE_FORCE_INTRINSICS

/**
 * The orange_spinlock_t type.
 */
typedef struct orange_spinlock {
	volatile int locked; /**< lock status 0 = unlocked, 1 = locked */
} orange_spinlock_t;

/**
 * A static spinlock initializer.
 */
#define ORANGE_SPINLOCK_INITIALIZER                                                                                                                            \
	{                                                                                                                                                          \
		0                                                                                                                                                      \
	}

/**
 * Initialize the spinlock to an unlocked state.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void orange_spinlock_init_ex(orange_spinlock_t* sl, char* name)
{
	sl->locked = 0;
}

static inline void orange_spinlock_init(orange_spinlock_t* sl, char* name)
{
	sl->locked = 0;
}

static __inline__ void orange_spinlock_destroy(orange_spinlock_t* lock)
{
	return;
}

/**
 * Take the spinlock.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static void orange_spinlock_lock(orange_spinlock_t* sl)
{
	int lock_val = 1;

	if (sl == NULL) {
		orange_log(ORANGE_LOG_ERR, "%s:%d spinlock is null.\n", __func__, __LINE__);
		return;
	}
	__asm __volatile("1:\n"
					 "xchg %[locked], %[lv]\n"
					 "test %[lv], %[lv]\n"
					 "jz 3f\n"
					 "2:\n"
					 "pause\n"
					 "cmpl $0, %[locked]\n"
					 "jnz 2b\n"
					 "jmp 1b\n"
					 "3:\n"
					 : [locked] "=m"(sl->locked), [lv] "=q"(lock_val)
					 : "[lv]"(lock_val)
					 : "memory");
}

/**
 * Release the spinlock.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void orange_spinlock_unlock(orange_spinlock_t* sl)
{
	int unlock_val = 0;

	if (sl == NULL) {
		orange_log(ORANGE_LOG_ERR, "%s:%d spinlock is null.\n", __func__, __LINE__);
		return;
	}

	__asm __volatile("xchg %[locked], %[ulv]\n" : [locked] "=m"(sl->locked), [ulv] "=q"(unlock_val) : "[ulv]"(unlock_val) : "memory");
}

/**
 * Try to take the lock.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int orange_spinlock_trylock(orange_spinlock_t* sl)
{
	int lockval = 1;

	if (sl == NULL) {
		orange_log(ORANGE_LOG_ERR, "%s:%d spinlock is null.\n", __func__, __LINE__);
		goto exit;
	}

	__asm __volatile("xchg %[locked], %[lockval]" : [locked] "=m"(sl->locked), [lockval] "=q"(lockval) : "[lockval]"(lockval) : "memory");

exit:
	return (lockval == 0);
}

/**
 * Test if the lock is taken.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is currently taken; 0 otherwise.
 */
static inline int orange_spinlock_is_locked(orange_spinlock_t* sl)
{
	return sl->locked;
}

#else

#include <orange/base/orange_atomic.h>

typedef struct orange_spinlock {
	orange_atomic_t var;
} orange_spinlock_t;

static __inline__ void orange_spinlock_init_ex(struct orange_spinlock* lock, char* name, int type)
{
	orange_atomic_set(&lock->var, 0);
}

static __inline__ void orange_spinlock_init(struct orange_spinlock* lock, char* name)
{
	orange_atomic_set(&lock->var, 0);
}

static __inline__ void orange_spinlock_destroy(struct orange_spinlock* lock)
{
	return;
}

static __inline__ void orange_spinlock_lock(struct orange_spinlock* lock)
{
	while (__sync_lock_test_and_set(&sl->locked, 1)) {
		while (sl->locked) {
			orange_cpu_spinwait();
		}
	}
}

static __inline__ void orange_spinlock_unlock(struct orange_spinlock* lock)
{
	orange_barrier();
	orange_atomic_set(&lock->var, 0);
}

static __inline__ int orange_spinlock_trylock(struct orange_spinlock* lock)
{
	return orange_atomic_cmpset(&lock->var, 0, 1);
}

static __inline__ int orange_spinlock_is_locked(struct orange_spinlock* lock)
{
	return (orange_atomic_read(&lock->var) == 0) ? 0 : 1;
}

#endif /* ORANGE_FORCE_INTRINSICS */
#endif /* _KERNEL */

/**
 * The orange_spinlock_recursive_t type.
 */
typedef struct {
	orange_spinlock_t sl;	/**< the actual spinlock */
	volatile uint8_t  user;  /**< core id using lock, -1 for unused */
	volatile uint32_t count; /**< count of time this lock has been called */
} orange_spinlock_recursive_t;

/**
 * A static recursive spinlock initializer.
 */
#define ORANGE_SPINLOCK_RECURSIVE_INITIALIZER                                                                                                                  \
	{                                                                                                                                                          \
		ORANGE_SPINLOCK_INITIALIZER, -1, 0                                                                                                                     \
	}

/**
 * Initialize the recursive spinlock to an unlocked state.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline void orange_spinlock_recursive_init(orange_spinlock_recursive_t* slr, char* name)
{
	orange_spinlock_init(&slr->sl, name);
	slr->user  = -1;
	slr->count = 0;
}

/**
 * Take the recursive spinlock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline void orange_spinlock_recursive_lock(orange_spinlock_recursive_t* slr)
{
	uint8_t id = ORANGE_CPU_ID;

	if (slr->user != id) {
		orange_spinlock_lock(&slr->sl);
		slr->user = id;
	}
	slr->count++;
}
/**
 * Release the recursive spinlock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline void orange_spinlock_recursive_unlock(orange_spinlock_recursive_t* slr)
{
	if (--(slr->count) == 0) {
		slr->user = -1;
		orange_spinlock_unlock(&slr->sl);
	}
}

/**
 * Try to take the recursive lock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int orange_spinlock_recursive_trylock(orange_spinlock_recursive_t* slr)
{
	uint8_t id = ORANGE_CPU_ID;

	if (slr->user != id) {
		if (orange_spinlock_trylock(&slr->sl) == 0)
			return 0;
		slr->user = id;
	}

	slr->count++;

	return 1;
}

#endif /* __ORANGE_SPINLOCK_H__ */
