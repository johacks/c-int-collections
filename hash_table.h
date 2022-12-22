/*
Copyright (c) 2005-2008, Simon Howard
Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice appear
in all copies.
THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "list.h"

#ifndef ALGORITHM_HASH_TABLE_H
#define ALGORITHM_HASH_TABLE_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct _HashTable HashTable;

    typedef struct _HashTableIterator HashTableIterator;

    typedef struct _HashTableEntry HashTableEntry;

    typedef struct _HashTablePair
    {
        int key;
        List *values;
    } HashTablePair;

    struct _HashTableIterator
    {
        HashTable *hash_table;
        HashTableEntry *next_entry;
        unsigned int next_chain;
    };

    struct _HashTableEntry
    {
        HashTablePair pair;
        HashTableEntry *next;
    };

    struct _HashTable
    {
        HashTableEntry **table;
        unsigned int table_size;
        unsigned int entries;
        unsigned int prime_index;
    };

    /**
     * A null @ref HashTableValue.
     */

#define HASH_TABLE_NULL ((void *)0)

    /**
     * Type of function used to free values when entries are removed from a
     * hash table.
     */

    typedef void (*HashTableValueFreeFunc)(void *value);

    HashTable *hash_table_new();

    void hash_table_free(HashTable *hash_table);

    int hash_table_insert(HashTable *hash_table, int key, int value);

    List *hash_table_lookup(HashTable *hash_table, int key);

    int hash_table_remove(HashTable *hash_table, int key);

    unsigned int hash_table_num_entries(HashTable *hash_table);

    void hash_table_iterate(HashTable *hash_table, HashTableIterator *iter);

    int hash_table_iter_has_more(HashTableIterator *iterator);

    HashTablePair hash_table_iter_next(HashTableIterator *iterator);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef ALGORITHM_HASH_TABLE_H */