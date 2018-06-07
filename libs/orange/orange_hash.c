#include "orange_hash.h"
#include <errno.h>

static inline uint32_t __orange_hash_swap32(uint32_t x)
{
	return ((x << 24) & 0xff000000) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff);
}

uint32_t orange_hash32(const void* input, size_t length, unsigned int seed)
{
	return 0;
}

struct orange_hash32_state* orange_hash32_create_state(void)
{
	return NULL;
}

int orange_hash32_free_state(struct orange_hash32_state* state_ptr)
{
	return 0;
}

void orange_hash32_copy_state(struct orange_hash32_state* dst_state, const struct orange_hash32_state* src_state)
{
	return;
}

int orange_hash32_reset(struct orange_hash32_state* state_ptr, unsigned int seed)
{
	return 0;
}

int orange_hash32_update(struct orange_hash32_state* state_ptr, const void* input, size_t length)
{
	return 0;
}

uint32_t orange_hash32_digest(const struct orange_hash32_state* state_ptr)
{
	return 0;
}

void orange_hash32_canonical_from_hash(struct orange_hash32_canonical* dst, uint32_t hash)
{
	return;
}

uint32_t orange_hash32_hash_from_canonical(const struct orange_hash32_canonical* src)
{
	return 0;
}

uint64_t orange_hash64(const void* input, size_t length, unsigned long long seed)
{
	return 0;
}

struct orange_hash64_state* orange_hash64_create_state(void)
{
	return NULL;
}

int orange_hash64_free_state(struct orange_hash64_state* state_ptr)
{
	return 0;
}

void orange_hash64_copy_state(struct orange_hash64_state* dst_state, const struct orange_hash64_state* src_state)
{
	return;
}

int orange_hash64_reset(struct orange_hash64_state* state_ptr, unsigned long long seed)
{
	return 0;
}

int orange_hash64_update(struct orange_hash64_state* state_ptr, const void* input, size_t length)
{
	return 0;
}

uint32_t orange_hash64_digest(const struct orange_hash64_state* state_ptr)
{
	return 0;
}

void orange_hash64_canonical_from_hash(struct orange_hash64_canonical* dst, uint64_t hash)
{
	return;
}

uint32_t orange_hash64_hash_from_canonical(const struct orange_hash64_canonical* src)
{
	return 0;
}
