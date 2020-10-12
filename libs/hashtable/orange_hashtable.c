#include "orange_hashtable.h"
#include "../orange/orange_log.h"
#include "../orange/orange_mutex.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_thread.h"
#include "../orange/orange_tree.h"
#include "orange_hashtable_version.h"

ORANGE_VERSION_GENERATE(orange_hashtable, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

orange_hash_table_t* orange_hashtable_create(int modulus, orange_hashtable_hashfunc* hasher, 
        orange_hashtable_keycmp* cmp, orange_hashtable_hashfree* keyfree, orange_hashtable_hashfree* datafree)
{
    orange_hash_table_t* new = malloc(sizeof(struct orange_hash_table));
    if (NULL == new) {
        return NULL;
    }

    new->modulus   = modulus;
    new->hasher    = hasher;
    new->keycmp    = cmp;
    new->keyfree   = keyfree;
    new->datafree  = datafree;
    new->items     = malloc(modulus * sizeof(struct orange_hashtable_item));
    if (NULL == new->items) {
        free(new);
        return NULL;
    }
    return new;
}

void orange_hashtable_destroy(orange_hash_table_t* tbl)
{
    orange_hashtable_item_t *item, *next;
    int       slot;
    if (NULL == tbl) {
        return;
    }

    for (slot = 0; slot < tbl->modulus; slot++) {
        for (item = tbl->items[slot]; item;) {
            next = item->next;
            if (tbl->keyfree) {
                tbl->keyfree((void*)item->key);
            }
            if (tbl->datafree) {
                tbl->datafree(item->data);
            }
            free(item);
            item = next;
        }
    }

    free(tbl);
}

int orange_hashtable_add(const void* key, void* data, orange_hash_table_t* tbl)
{
    orange_hashtable_item_t* new = malloc(sizeof(orange_hashtable_item_t));
    orange_hashtable_item_t** item;
    int        slot;
    if (NULL == new) {
        return 1;
    }

    new->key  = key;
    new->data = data;
    slot      = tbl->hasher(key) % tbl->modulus;
    for (item = &tbl->items[slot]; *item; item = &(*item)->next);
    *item = new;

    return 0;
}

void orange_hashtable_remove(const void* key, orange_hash_table_t* tbl)
{
    orange_hashtable_item_t **pitem, *item;
    int        slot;
    slot = tbl->hasher(key) % tbl->modulus;
    for (pitem = &tbl->items[slot]; *pitem; pitem = &(*pitem)->next) {
        if (0 == tbl->keycmp(key, (*pitem)->key)) {
            item  = *pitem;
            *pitem = (*pitem)->next;
            if (tbl->keyfree) {
                tbl->keyfree((void*)item->key);
            }
            if (tbl->datafree) {
                tbl->datafree(item->data);
            }
            if (!tbl->use_arena) {
                free(item);
            }
            break;
        }
    }
}

void* orange_hashtable_find(const void* key, orange_hash_table_t* tbl)
{
    int       slot = tbl->hasher(key) % tbl->modulus;
    orange_hashtable_item_t* item;
    for (item = tbl->items[slot]; item; item = item->next) {
        if (0 == tbl->keycmp(key, item->key)) {
            return item->data;
        }
    }
    return NULL;
}

static void __orange_hashtable_iter_next_slot(orange_hash_table_t* tbl)
{
    while (tbl->iter.next == NULL) {
        tbl->iter.slot++;
        if (tbl->iter.slot == tbl->modulus) {
            break;
        }
        tbl->iter.next = tbl->items[tbl->iter.slot];
    }

    return;
}

void orange_hashtable_iter_init(orange_hash_table_t* tbl)
{
    tbl->iter.slot = 0;
    tbl->iter.next = tbl->items[tbl->iter.slot];
    if (NULL == tbl->iter.next) {
        __orange_hashtable_iter_next_slot(tbl);
    }
    return;
}

void* orange_hashtable_iterate(orange_hash_table_t* tbl)
{
    orange_hashtable_item_t* this = tbl->iter.next;
    if (this) {
        tbl->iter.next = this->next;
        if (NULL == tbl->iter.next) {
            __orange_hashtable_iter_next_slot(tbl);
        }
    }
    return this ? this->data : NULL;
}

static int __orange_hashtable_module_init(void)
{
	snprintf(orange_hashtable_description, 127, "Orange Hash Table Module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_hashtable_version),
			 orange_version_type(orange_hashtable_version_type), orange_hashtable_build_num, orange_hashtable_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_hashtable_description);

	return 0;
}

static void __orange_hashtable_module_fini(void)
{
	orange_log(ORANGE_LOG_INFO, "Orange Hash Table Module unloaded.\n");

	return;
}

static int orange_hashtable_modhashtable(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_hashtable_module_init();
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_hashtable_module_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_hashtable_mod = {"orange_hashtable", orange_hashtable_modhashtable, 0};

ORANGE_DECLARE_MODULE(orange_hashtable, orange_hashtable_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_hashtable, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_hashtable);
