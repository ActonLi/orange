#include "orange_autoid.h"
#include "orange_log.h"

#if 0
#define DEBUGP printf
#define DEBUGP_CLR printf
#else
#define DEBUGP(format, args...)
#define DEBUGP_CLR(format, args...)
#endif

#define ORANGE_AUTOID_MAGIC 0x2f2e2f2e

typedef struct orange_autoid {
	uint32_t		magic;
	uint32_t		max_id;
	orange_atomic_t use_count;
	uint16_t		levels;
	uint16_t		indexs_per_bit;
	uint16_t		indexs_per_bit_shift;
	uint16_t		index_offsets[ORANGE_AUTOID_LEVELS_MAX];
	uint32_t		index_nums[ORANGE_AUTOID_LEVELS_MAX];
	int				indexs[0];
} orange_autoid;

#define orange_autoid_malloc(size) malloc(size)
static __inline__ struct orange_autoid* orange_autoid_zalloc(int size)
{
	struct orange_autoid* autoid = malloc(size);
	if (autoid != NULL) {
		memset(autoid, 0, size);
	}
	return autoid;
}

#define orange_autoid_free(p) free(p)

#define FFS_BIT_IS_SET(v, b) (((v) & (1 << ((b) -1))) != 0)
#define FFS_BIT_SET(v, b) (v) = ((v) | (1 << ((b) -1)))
#define FFS_BIT_CLR(v, b) (v) = ((v) & (~(1 << ((b) -1))))

static int orange_autoid_retry_count = 16;

void orange_autoid_dump(struct orange_autoid* autoid, int show_bitmap, int (*print)(const char* fmt, ...))
{
	int		 i, j, index_nums, offset;
	uint32_t value;

	print("-------------------autoid dump---------------------\n");
	print("magic: %x, memory size %lu\n", autoid->magic, orange_autoid_memsize(autoid));
	print("max_id: %u, use_count: %u, level: %u\n", autoid->max_id, orange_atomic_read(&autoid->use_count), autoid->levels);
	print("indexs_per_bit: %u, indexs_per_bit_shift: %u\n", autoid->indexs_per_bit, autoid->indexs_per_bit_shift);
	for (i = 0; i < ORANGE_AUTOID_LEVELS_MAX; i++) {
		print("level %d, index offset %u, index num %u\n", i, autoid->index_offsets[i], autoid->index_nums[i]);

		if (!show_bitmap) {
			continue;
		}
		index_nums = autoid->index_nums[i];
		offset	 = autoid->index_offsets[i];
		for (j = 0; j < index_nums; j++) {
			value = *(autoid->indexs + offset + j);
			/* unused bits */
			if (value > 0) {
				print("[%d]: %x\t", j, value);
			}
		}
		print("\n");
	}
}

uint64_t orange_autoid_memsize(struct orange_autoid* autoid)
{
	if (autoid == NULL || autoid->magic != ORANGE_AUTOID_MAGIC) {
		orange_log(ORANGE_LOG_ERR, "%s: autoid is invaild, autoid: %p, magic: %x/%x\n", __func__, autoid, (autoid == NULL) ? 0 : autoid->magic,
				   ORANGE_AUTOID_MAGIC);
		return 0;
	}

	return (sizeof(struct orange_autoid) + (autoid->index_nums[0] + autoid->index_nums[1] + autoid->index_nums[2]) * sizeof(int));
}

struct orange_autoid* orange_autoid_create_arg(int max, int indexs_per_bit)
{
	struct orange_autoid* autoid			   = NULL;
	int					  primary_index_nums   = 0;
	int					  secondary_index_nums = 0;
	int					  third_index_nums	 = 0;
	int					  indexs_per_bit_shift = 0;
	int					  index_nums		   = 0;
	int					  levels			   = 0;

	if (indexs_per_bit <= 0) {
		indexs_per_bit = ORANGE_AUTOID_INDEXS_PER_BIT_MAX;
	}

	for (indexs_per_bit_shift = ORANGE_AUTOID_BITS_PER_INDEX_SHIFT; indexs_per_bit_shift < 16; indexs_per_bit_shift++) {
		if ((0x01 << indexs_per_bit_shift) >= indexs_per_bit) {
			indexs_per_bit = (0x01 << indexs_per_bit_shift);
			break;
		}
	}
	if ((0x01 << indexs_per_bit_shift) < indexs_per_bit) {
		goto exit;
	}

	if (unlikely(max > (indexs_per_bit * indexs_per_bit * indexs_per_bit))) {
		goto exit;
	}

	max = (max % ORANGE_AUTOID_BITS_PER_INDEX) == 0 ? max : ((max / ORANGE_AUTOID_BITS_PER_INDEX) + 1) * ORANGE_AUTOID_BITS_PER_INDEX;
	if (max <= (ORANGE_AUTOID_BITS_PER_INDEX * 8)) {
		primary_index_nums = max / ORANGE_AUTOID_BITS_PER_INDEX;
		levels			   = 1;
	} else if (max <= indexs_per_bit * indexs_per_bit) {
		secondary_index_nums = max / ORANGE_AUTOID_BITS_PER_INDEX;
		primary_index_nums =
			(secondary_index_nums % indexs_per_bit) == 0 ? (secondary_index_nums / indexs_per_bit) : ((secondary_index_nums / indexs_per_bit) + 1);
		max	= secondary_index_nums * ORANGE_AUTOID_BITS_PER_INDEX;
		levels = 2;
	} else {
		third_index_nums = max / ORANGE_AUTOID_BITS_PER_INDEX;

		secondary_index_nums = (third_index_nums % indexs_per_bit) == 0 ? (third_index_nums / indexs_per_bit) : ((third_index_nums / indexs_per_bit) + 1);

		primary_index_nums =
			(secondary_index_nums % indexs_per_bit) == 0 ? (secondary_index_nums / indexs_per_bit) : ((secondary_index_nums / indexs_per_bit) + 1);

		max	= third_index_nums * ORANGE_AUTOID_BITS_PER_INDEX;
		levels = 3;
	}

	if (primary_index_nums == 0) {
		goto exit;
	}

	index_nums = primary_index_nums + secondary_index_nums + third_index_nums;
	autoid	 = orange_autoid_zalloc(sizeof(struct orange_autoid) + index_nums * sizeof(int));
	if (unlikely(autoid == NULL)) {
		goto exit;
	}

	autoid->magic  = ORANGE_AUTOID_MAGIC;
	autoid->max_id = max;
	orange_atomic_set(&autoid->use_count, 0);
	autoid->index_nums[0]		 = primary_index_nums;
	autoid->index_nums[1]		 = secondary_index_nums;
	autoid->index_nums[2]		 = third_index_nums;
	autoid->indexs_per_bit		 = indexs_per_bit;
	autoid->indexs_per_bit_shift = indexs_per_bit_shift;
	if (secondary_index_nums) {
		autoid->index_offsets[1] = primary_index_nums;
	}
	if (third_index_nums) {
		autoid->index_offsets[2] = primary_index_nums + secondary_index_nums;
	}
	autoid->levels = levels;
	memset(((char*) autoid->indexs), 0xff, index_nums * sizeof(int));
#ifdef ORANGE_AUTOID_DUMP
	for (int i = 0; i < index_nums; i++) {
		DEBUGP("%d: \t%x\n", i, autoid->indexs[i]);
	}
#endif

exit:
	DEBUGP("%s finished, autoid: %p\n", __func__, autoid);
	return autoid;
}

void orange_autoid_destroy(struct orange_autoid* autoid)
{
	DEBUGP("%s begin: autoid: %p\n", __func__, autoid);

	if (autoid != NULL) {
		orange_autoid_free(autoid);
	}
	DEBUGP("%s finished\n", __func__);
}

static int __orange_autoid_get(struct orange_autoid* autoid)
{
	int		 id = ORANGE_AUTOID_NONE;
	uint32_t value;
	uint32_t new_value;
	int		 free_bit;
	int		 index_num   = 0;
	int		 level		 = 0;
	int		 index_begin = 0;
	int		 index_end   = autoid->index_nums[level];

	while (id == ORANGE_AUTOID_NONE && level < autoid->levels) {
		free_bit = 0;

		while (index_num < index_end) {
			value = *(autoid->indexs + autoid->index_offsets[level] + index_num);

			if (value == 0 || (free_bit = orange_ffsl(value)) == 0) {
				index_num++;
				continue;
			}

			level++;

			if (level == autoid->levels) {
				/* success found free bit */
				new_value = value;
				FFS_BIT_CLR(new_value, free_bit);

				if (orange_atomic_cmpset((orange_atomic_t*) (autoid->indexs + autoid->index_offsets[level - 1] + index_num), value, new_value) == 1) {
					/* success compare and set alloced bit*/
					id = ((index_num << (ORANGE_AUTOID_BITS_PER_INDEX_SHIFT)) + (free_bit - 1));

					orange_atomic_inc(&autoid->use_count);

					break;
				}
			}

			if (level >= autoid->levels) {
				break;
			}

			index_begin = ((index_num << ORANGE_AUTOID_BITS_PER_INDEX_SHIFT) + (free_bit - 1))
						  << ((autoid->indexs_per_bit_shift - ORANGE_AUTOID_BITS_PER_INDEX_SHIFT));
			index_end = index_begin + autoid->indexs_per_bit / ORANGE_AUTOID_BITS_PER_INDEX;
			index_end = index_end >= autoid->index_nums[level] ? autoid->index_nums[level] : index_end;
			free_bit  = 0;
			index_num = index_begin;
		}

		if (free_bit == 0) {
			if (level > 0) {
				free_bit  = index_begin >> (autoid->indexs_per_bit_shift - ORANGE_AUTOID_BITS_PER_INDEX_SHIFT);
				index_num = free_bit >> ORANGE_AUTOID_BITS_PER_INDEX_SHIFT;
				free_bit  = free_bit % 32;
				level--;
				value	 = *(autoid->indexs + autoid->index_offsets[level] + index_num);
				new_value = value;
				FFS_BIT_CLR(new_value, free_bit + 1);

				orange_atomic_cmpset((orange_atomic_t*) (autoid->indexs + autoid->index_offsets[level] + index_num), value, new_value);

				level = autoid->levels;
			}

			break;
		}
	}

	DEBUGP("%s finished: id: %d\n", __func__, id);
	return id;
}

static void __orange_autoid_set(struct orange_autoid* autoid, int id)
{
	int index_num;
	int free_bit;
	int value;
	int new_value;
	int level;

	level = autoid->levels - 1;

	index_num = id >> ORANGE_AUTOID_BITS_PER_INDEX_SHIFT;

	free_bit = id % ORANGE_AUTOID_BITS_PER_INDEX + 1;

	value = *(autoid->indexs + autoid->index_offsets[level] + index_num);

	new_value = value;

	FFS_BIT_CLR(new_value, free_bit);

	if (orange_atomic_cmpset((orange_atomic_t*) (autoid->indexs + autoid->index_offsets[level] + index_num), value, new_value) == 1) {
		orange_atomic_inc(&autoid->use_count);
	}
	return;
}

static void __orange_autoid_put(struct orange_autoid* autoid, int id)
{
	int index_num;
	int free_bit;
	int value;
	int new_value;
	int level;

	level	 = autoid->levels - 1;
	index_num = id >> ORANGE_AUTOID_BITS_PER_INDEX_SHIFT;
	free_bit  = id % ORANGE_AUTOID_BITS_PER_INDEX + 1;

	while (level >= 0) {
		value	 = *(autoid->indexs + autoid->index_offsets[level] + index_num);
		new_value = value;
		FFS_BIT_SET(new_value, free_bit);

		if (new_value == value) {
			break;
		}

		while (1) {
			if (orange_atomic_cmpset((orange_atomic_t*) (autoid->indexs + autoid->index_offsets[level] + index_num), value, new_value) == 1) {
				free_bit  = index_num >> (autoid->indexs_per_bit_shift - ORANGE_AUTOID_BITS_PER_INDEX_SHIFT);
				index_num = free_bit >> ORANGE_AUTOID_BITS_PER_INDEX_SHIFT;
				free_bit  = free_bit % 32 + 1;
				level--;
				break;
			} else {
				/* 比较交换失败，需要重新计算value */
				value	 = *(autoid->indexs + autoid->index_offsets[level] + index_num);
				new_value = value;
				FFS_BIT_SET(new_value, free_bit);

				if (new_value == value) {
					break;
				}
			}
		}
	}

	if (level != autoid->levels - 1) {
		if (orange_atomic_read(&autoid->use_count) > 0) {
			orange_atomic_dec(&autoid->use_count);
		} else {
			orange_log(ORANGE_LOG_ERR, "%s: use_count is zero.\n", __func__);
		}
	}
}

int orange_autoid_unused_count_get(struct orange_autoid* autoid)
{
	if (autoid == NULL || autoid->magic != ORANGE_AUTOID_MAGIC) {
		return 0;
	}

	return (autoid->max_id - orange_atomic_read(&autoid->use_count));
}

int orange_autoid_get(struct orange_autoid* autoid)
{
	int id			= ORANGE_AUTOID_NONE;
	int retry_count = 0;

	if (autoid == NULL || autoid->magic != ORANGE_AUTOID_MAGIC) {
		orange_log(ORANGE_LOG_ERR, "%s: autoid is invaild, autoid: %p, magic: %x/%x\n", __func__, autoid, (autoid == NULL) ? 0 : autoid->magic,
				   ORANGE_AUTOID_MAGIC);
		return id;
	}

	while (likely(orange_atomic_read(&autoid->use_count) < autoid->max_id && retry_count < orange_autoid_retry_count)) {
		id = __orange_autoid_get(autoid);
		if (id != ORANGE_AUTOID_NONE) {
			id++;
			break;
		}
		retry_count++;
	}
	return id;
}

uint32_t orange_autoid_get_max_id(struct orange_autoid* autoid)
{
	if (autoid) {
		return autoid->max_id;
	}

	return 0;
}
void orange_autoid_set(struct orange_autoid* autoid, int id)
{
	if (autoid == NULL || autoid->magic != ORANGE_AUTOID_MAGIC) {
		orange_log(ORANGE_LOG_ERR, "%s: autoid is invaild, autoid: %p, magic: %x/%x\n", __func__, autoid, (autoid == NULL) ? 0 : autoid->magic,
				   ORANGE_AUTOID_MAGIC);
		return;
	}

	if (id <= 0 || id > autoid->max_id) {
		return;
	}
	__orange_autoid_set(autoid, id - 1);
}

void orange_autoid_put(struct orange_autoid* autoid, int id)
{
	if (autoid == NULL || autoid->magic != ORANGE_AUTOID_MAGIC) {
		orange_log(ORANGE_LOG_ERR, "%s: autoid is invaild, autoid: %p, magic: %x/%x\n", __func__, autoid, (autoid == NULL) ? 0 : autoid->magic,
				   ORANGE_AUTOID_MAGIC);
		return;
	}

	if (id <= 0 || id > autoid->max_id
		/* || orange_atomic_read(&autoid->use_count) == 0 */) {
		return;
	}

	__orange_autoid_put(autoid, id - 1);
}

ssize_t orange_autoid_total_size(struct orange_autoid* autoid)
{
	int index_nums = 0;
	int i;
	for (i = 0; i < autoid->levels; i++) {
		index_nums += autoid->index_nums[i];
	}
	return sizeof(struct orange_autoid) + (index_nums * sizeof(int));
}

void orange_autoid_reinit(struct orange_autoid* autoid)
{
	int index_nums = 0;
	int i;

	for (i = 0; i < autoid->levels; i++) {
		index_nums += autoid->index_nums[i];
	}

	memset(((char*) autoid->indexs), 0xff, index_nums * sizeof(int));
	orange_atomic_set(&autoid->use_count, 0);
}

int orange_autoid_init(void)
{
	orange_autoid_retry_count = (ORANGE_CPU_NUMS * 2);

	DEBUGP("%s, orange_autoid_retry_count %d\n", __func__, orange_autoid_retry_count);
	return 0;
}

void orange_autoid_fini(void)
{
	return;
}
