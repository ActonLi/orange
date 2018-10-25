#pragma once
#include "orange.h"
#include "orange_atomic.h"

#define ORANGE_AUTOID_LEVELS_MAX 3

#define ORANGE_AUTOID_NONE -1
#define ORANGE_AUTOID_BITS_PER_INDEX 32
#define ORANGE_AUTOID_BITS_PER_INDEX_SHIFT 5
#define ORANGE_AUTOID_INDEXS_PER_BIT_MAX 512
#define ORANGE_AUTOID_SECONDARY_ID_MAX (512 * 512) /* 262144 */
#define ORANGE_AUTOID_MAX (512 * 512 * 512)		   /* 134217728, 134M */

struct orange_autoid;
extern struct orange_autoid* orange_autoid_create_arg(int max, int indexs_per_bit);

static __inline__ struct orange_autoid* orange_autoid_create(int max)
{
	return orange_autoid_create_arg(max, -1);
}

extern void orange_autoid_destroy(struct orange_autoid* autoid);
extern int orange_autoid_get(struct orange_autoid* autoid);
extern void orange_autoid_put(struct orange_autoid* autoid, int id);
extern void orange_autoid_set(struct orange_autoid* autoid, int id);
extern ssize_t orange_autoid_total_size(struct orange_autoid* autoid);
extern void orange_autoid_reinit(struct orange_autoid* autoid);
extern int orange_autoid_unused_count_get(struct orange_autoid* autoid);

extern uint64_t orange_autoid_memsize(struct orange_autoid* autoid);
extern void orange_autoid_dump(struct orange_autoid* autoid, int show_bitmap, int (*print)(const char* fmt, ...));
extern uint32_t orange_autoid_get_max_id(struct orange_autoid* autoid);

extern int  orange_autoid_init(void);
extern void orange_autoid_fini(void);
