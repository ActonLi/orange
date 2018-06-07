#pragma once
#include "orange.h"

typedef struct orange_hash32_state {
	uint32_t total_len_32;
	uint32_t large_len;
	uint32_t v1;
	uint32_t v2;
	uint32_t v3;
	uint32_t v4;
	uint32_t mem32[4];
	uint32_t memsize;
	uint32_t reserved;
} orange_hash32_state_t;

typedef struct orange_hash64_state {
	uint64_t total_len;
	uint64_t v1;
	uint64_t v2;
	uint64_t v3;
	uint64_t v4;
	uint64_t mem64[4];
	uint32_t memsize;
	uint32_t reserved[2];
} orange_hash64_state_t;

typedef struct orange_hash32_canonical {
	unsigned char digest[4];
} orange_hash32_canonical_t;

typedef struct orange_hash64_canonical {
	unsigned char digest[8];
} orange_hash64_canonical_t;

uint32_t orange_hash32(const void* input, size_t length, unsigned int seed);
struct orange_hash32_state* orange_hash32_create_state(void);
int orange_hash32_free_state(struct orange_hash32_state* state_ptr);
void orange_hash32_copy_state(struct orange_hash32_state* dst_state, const struct orange_hash32_state* src_state);
int orange_hash32_reset(struct orange_hash32_state* state_ptr, unsigned int seed);
int orange_hash32_update(struct orange_hash32_state* state_ptr, const void* input, size_t length);
uint32_t orange_hash32_digest(const struct orange_hash32_state* state_ptr);
void orange_hash32_canonical_from_hash(struct orange_hash32_canonical* dst, uint32_t hash);
uint32_t orange_hash32_hash_from_canonical(const struct orange_hash32_canonical* src);

uint64_t orange_hash64(const void* input, size_t length, unsigned long long seed);
struct orange_hash64_state* orange_hash64_create_state(void);
int orange_hash64_free_state(struct orange_hash64_state* state_ptr);
void orange_hash64_copy_state(struct orange_hash64_state* dst_state, const struct orange_hash64_state* src_state);
int orange_hash64_reset(struct orange_hash64_state* state_ptr, unsigned long long seed);
int orange_hash64_update(struct orange_hash64_state* state_ptr, const void* input, size_t length);
uint32_t orange_hash64_digest(const struct orange_hash64_state* state_ptr);
void orange_hash64_canonical_from_hash(struct orange_hash64_canonical* dst, uint64_t hash);
uint32_t orange_hash64_hash_from_canonical(const struct orange_hash64_canonical* src);
