#ifndef __ORANGE_BITOPS_H__
#define __ORANGE_BITOPS_H__

/* SMP sense */
#define LOCK_PREFIX "lock ; "

/* gcc sense */
#define BITOP_ADDR(x) "+m"(*(__volatile long*) (x))
#define ADDR BITOP_ADDR(addr)

/*
 * We do the locked ops that don't return the old value as
 * a mask operation on a byte.
 */
#define IS_IMMEDIATE(nr) (__builtin_constant_p(nr))
#define CONST_MASK_ADDR(nr, addr) BITOP_ADDR((addr) + ((nr) >> 3))
#define CONST_MASK(nr) (1 << ((nr) &7))

/*
 * How to treat the bitops index for bitops instructions. Casting this
 * to unsigned long correctly generates 64-bit operations on 64 bits.
 */
#ifdef __i386__
#define IDX(nr) "Ir"(nr)
#define BITS_PER_LONG 32
#define SHIFT_NUM_PER_LONG 5
#else
#define IDX(nr) "Jr"(nr)
#define BITS_PER_LONG 64
#define SHIFT_NUM_PER_LONG 6
#endif

/*
 * set_bit - Atomically set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * This function is atomic and may not be reordered.  See __set_bit()
 * if you do not require the atomic guarantees.
 *
 * Note: there are no guarantees that this function will not be reordered
 * on non x86 architectures, so if you are writing portable code,
 * make sure not to rely on its reordering guarantees.
 *
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void orange_atomic_bit_set(unsigned long nr, __volatile unsigned long* addr)
{
	if (IS_IMMEDIATE(nr)) {
		__asm __volatile(LOCK_PREFIX "orb %1,%0" : CONST_MASK_ADDR(nr, addr) : "iq"((uint8_t) CONST_MASK(nr)) : "memory");
	} else {
		__asm __volatile(LOCK_PREFIX "bts %1,%0" : BITOP_ADDR(addr) : IDX(nr) : "memory");
	}
}

/*
 * clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * clear_bit() is atomic and may not be reordered.  However, it does
 * not contain a memory barrier, so if it is used for locking purposes,
 * you should call smp_mb__before_clear_bit() and/or smp_mb__after_clear_bit()
 * in order to ensure changes are visible on other processors.
 */
static inline void orange_atomic_bit_clear(unsigned long nr, volatile unsigned long* addr)
{
	if (IS_IMMEDIATE(nr)) {
		__asm __volatile(LOCK_PREFIX "andb %1,%0" : CONST_MASK_ADDR(nr, addr) : "iq"((uint8_t) ~CONST_MASK(nr)));
	} else {
		__asm __volatile(LOCK_PREFIX "btr %1,%0" : BITOP_ADDR(addr) : IDX(nr));
	}
}

/*
 * change_bit - Toggle a bit in memory
 * @nr: Bit to change
 * @addr: Address to start counting from
 *
 * change_bit() is atomic and may not be reordered.
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void orange_atomic_bit_change(unsigned long nr, volatile unsigned long* addr)
{
	if (IS_IMMEDIATE(nr)) {
		__asm __volatile(LOCK_PREFIX "xorb %1,%0" : CONST_MASK_ADDR(nr, addr) : "iq"((uint8_t) CONST_MASK(nr)));
	} else {
		__asm __volatile(LOCK_PREFIX "btc %1,%0" : BITOP_ADDR(addr) : IDX(nr));
	}
}

/*
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static inline int orange_atomic_bit_test_and_set(unsigned long nr, volatile unsigned long* addr)
{
	int oldbit;

	__asm __volatile(LOCK_PREFIX "bts %2,%1\n\t"
								 "sbb %0,%0"
					 : "=r"(oldbit), ADDR
					 : IDX(nr)
					 : "memory");

	return oldbit;
}

/*
 * test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static inline int orange_atomic_bit_test_and_clear(unsigned long nr, volatile unsigned long* addr)
{
	int oldbit;

	__asm __volatile(LOCK_PREFIX "btr %2,%1\n\t"
								 "sbb %0,%0"
					 : "=r"(oldbit), ADDR
					 : IDX(nr)
					 : "memory");

	return oldbit;
}

/*
 * test_and_change_bit - Change a bit and return its old value
 * @nr: Bit to change
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static inline int orange_test_and_change_bit(unsigned long nr, volatile unsigned long* addr)
{
	int oldbit;

	__asm __volatile(LOCK_PREFIX "btc %2,%1\n\t"
								 "sbb %0,%0"
					 : "=r"(oldbit), ADDR
					 : IDX(nr)
					 : "memory");

	return oldbit;
}

static inline int orange_atomic_bit_constant_test(unsigned long nr, const volatile unsigned long* addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) & (addr[nr / BITS_PER_LONG])) != 0;
}

/* XXX */
static inline int orange_atomic_bit_variable_test(unsigned long nr, unsigned long* addr)
{
	int oldbit;

	__asm __volatile("bt %2,%1\n\t"
					 "sbb %0,%0"
					 : "=r"(oldbit)
					 : "m"(*(unsigned long*) addr), IDX(nr));

	return oldbit;
}

#define orange_atomic_bit_test(nr, addr)                                                                                                                       \
	(__builtin_constant_p((nr)) ? orange_atomic_bit_constant_test((nr), (addr)) : orange_atomic_bit_variable_test((nr), (addr)))

/*
 * __ffs - find first set bit in word
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long orange_ffs(unsigned long word)
{
	__asm("rep; bsf %1,%0" : "=r"(word) : "rm"(word));
	return word;
}

/*
 * ffz - find first zero bit in word
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
static inline unsigned long orange_ffz(unsigned long word)
{
	__asm("rep; bsf %1,%0" : "=r"(word) : "r"(~word));
	return word;
}

/*
 * __fls: find last set bit in word
 * @word: The word to search
 *
 * Undefined if no set bit exists, so code should check against 0 first.
 */
static inline unsigned long orange_fls(unsigned long word)
{
	__asm("bsr %1,%0" : "=r"(word) : "rm"(word));
	return word;
}

#undef ADDR

/* TODO */
#ifdef __NOT_ASSURED__
/*
 * ffs - find first set bit in word
 * @x: the word to search
 *
 * This is defined the same way as the libc and compiler builtin ffs
 * routines, therefore differs in spirit from the other bitops.
 *
 * ffs(value) returns 0 if value is 0 or the position of the first
 * set bit if value is nonzero. The first (least significant) bit
 * is at position 1.
 */
static inline int orange_ffs(int x)
{
	int r;

#ifdef CONFIG_X86_64
	/*
	 * AMD64 says BSFL won't clobber the dest reg if x==0; Intel64 says the
	 * dest reg is undefined if x==0, but their CPU architect says its
	 * value is written to set it to the same as before, except that the
	 * top 32 bits will be cleared.
	 *
	 * We cannot do this on 32 bits because at the very least some
	 * 486 CPUs did not behave this way.
	 */
	__asm("bsfl %1,%0" : "=r"(r) : "rm"(x), "0"(-1));
#elif defined(CONFIG_X86_CMOV)
	__asm("bsfl %1,%0\n\t"
		  "cmovzl %2,%0"
		  : "=&r"(r)
		  : "rm"(x), "r"(-1));
#else
	__asm("bsfl %1,%0\n\t"
		  "jnz 1f\n\t"
		  "movl $-1,%0\n"
		  "1:"
		  : "=r"(r)
		  : "rm"(x));
#endif
	return r + 1;
}

/*
 * fls - find last set bit in word
 * @x: the word to search
 *
 * This is defined in a similar way as the libc and compiler builtin
 * ffs, but returns the position of the most significant set bit.
 *
 * fls(value) returns 0 if value is 0 or the position of the last
 * set bit if value is nonzero. The last (most significant) bit is
 * at position 32.
 */
static inline int orange_fls(int x)
{
	int r;

#ifdef CONFIG_X86_64
	/*
	 * AMD64 says BSRL won't clobber the dest reg if x==0; Intel64 says the
	 * dest reg is undefined if x==0, but their CPU architect says its
	 * value is written to set it to the same as before, except that the
	 * top 32 bits will be cleared.
	 *
	 * We cannot do this on 32 bits because at the very least some
	 * 486 CPUs did not behave this way.
	 */
	__asm("bsrl %1,%0" : "=r"(r) : "rm"(x), "0"(-1));
#elif defined(CONFIG_X86_CMOV)
	__asm("bsrl %1,%0\n\t"
		  "cmovzl %2,%0"
		  : "=&r"(r)
		  : "rm"(x), "rm"(-1));
#else
	__asm("bsrl %1,%0\n\t"
		  "jnz 1f\n\t"
		  "movl $-1,%0\n"
		  "1:"
		  : "=r"(r)
		  : "rm"(x));
#endif
	return r + 1;
}

/*
 * fls64 - find last set bit in a 64-bit word
 * @x: the word to search
 *
 * This is defined in a similar way as the libc and compiler builtin
 * ffsll, but returns the position of the most significant set bit.
 *
 * fls64(value) returns 0 if value is 0 or the position of the last
 * set bit if value is nonzero. The last (most significant) bit is
 * at position 64.
 */
#ifdef CONFIG_X86_64
static inline int orange_fls64(__u64 x)
{
	int bitpos = -1;
	/*
	 * AMD64 says BSRQ won't clobber the dest reg if x==0; Intel64 says the
	 * dest reg is undefined if x==0, but their CPU architect says its
	 * value is written to set it to the same as before.
	 */
	__asm("bsrq %1,%q0" : "+r"(bitpos) : "rm"(x));
	return bitpos + 1;
}
#endif

#endif

#define orange_ffsl(x) __builtin_ffsl(x)

#endif
