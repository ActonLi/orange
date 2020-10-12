#pragma once

#include "../orange/orange.h"
#include "../orange/orange_module.h"

ORANGE_VERSION_TYPE(orange_hashtable);
ORANGE_DEFINE_MODULE_EXTENSION(orange_hashtable);

typedef struct orange_hashtable_item {
    const void*       key;
    void*             data;
    struct orange_hashtable_item* next;
} orange_hashtable_item_t;

typedef unsigned int orange_hashtable_hashfunc(const void* key);
typedef int          orange_hashtable_keycmp(const void* a, const void* b);
typedef void         orange_hashtable_hashfree(void* p);

typedef struct orange_hash_table {
    unsigned int                        modulus;
    struct orange_hashtable_item**      items;
    orange_hashtable_hashfunc*          hasher;
    orange_hashtable_keycmp*            keycmp;
    int                                 use_arena;
    orange_hashtable_hashfree*          keyfree;
    orange_hashtable_hashfree*          datafree;

    struct {
        struct orange_hashtable_item*   next;
        unsigned int                    slot;
    } iter;
} orange_hash_table_t;

orange_hash_table_t* orange_hashtable_create(int modulus, orange_hashtable_hashfunc*, 
        orange_hashtable_keycmp*, orange_hashtable_hashfree*, orange_hashtable_hashfree*);
void orange_hashtable_destroy(orange_hash_table_t*);
int orange_hashtable_add(const void* key, void* data, orange_hash_table_t*);
void orange_hashtable_remove(const void* key,orange_hash_table_t* tbl);
void* orange_hashtable_find(const void* key, orange_hash_table_t*);
void orange_hashtable_iter_init(orange_hash_table_t*);
void* orange_hashtable_iterate(orange_hash_table_t*);


