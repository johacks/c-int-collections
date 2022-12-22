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


#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

static const unsigned int hash_table_primes[] = {
    193,
    389,
    769,
    1543,
    3079,
    6151,
    12289,
    24593,
    49157,
    98317,
    196613,
    393241,
    786433,
    1572869,
    3145739,
    6291469,
    12582917,
    25165843,
    50331653,
    100663319,
    201326611,
    402653189,
    805306457,
    1610612741,
};

static const unsigned int hash_table_num_primes = sizeof(hash_table_primes) / sizeof(int);

static int hash_table_allocate_table(HashTable *hash_table)
{
    unsigned int new_table_size;

    if (hash_table->prime_index < hash_table_num_primes)
    {
        new_table_size = hash_table_primes[hash_table->prime_index];
    }
    else
    {
        new_table_size = hash_table->entries * 10;
    }

    hash_table->table_size = new_table_size;

    /* Allocate the table and initialise to NULL for all entries */

    hash_table->table = calloc(hash_table->table_size,
                               sizeof(HashTableEntry *));

    return hash_table->table != NULL;
}

/* Free an entry, calling the free functions if there are any registered */

static void hash_table_free_entry(HashTable *hash_table, HashTableEntry *entry)
{
    list_free(entry->pair.values);
    free(entry);
}

HashTable *hash_table_init(HashTable *hash_table, HashTableValueFreeFunc value_free_func)
{

    hash_table->entries = 0;
    hash_table->prime_index = 0;

    /* Allocate the table */

    if (!hash_table_allocate_table(hash_table))
        return NULL;

    return hash_table;
}

void hash_table_free(HashTable *hash_table)
{
    HashTableEntry *rover;
    HashTableEntry *next;
    unsigned int i;

    /* Free all entries in all chains */

    for (i = 0; i < hash_table->table_size; ++i)
    {
        rover = hash_table->table[i];
        while (rover != NULL)
        {
            next = rover->next;
            hash_table_free_entry(hash_table, rover);
            rover = next;
        }
    }

    /* Free the table */

    free(hash_table->table);

}

static int hash_table_enlarge(HashTable *hash_table)
{
    HashTableEntry **old_table;
    unsigned int old_table_size;
    unsigned int old_prime_index;
    HashTableEntry *rover;
    HashTablePair *pair;
    HashTableEntry *next;
    unsigned int index;
    unsigned int i;

    /* Store a copy of the old table */

    old_table = hash_table->table;
    old_table_size = hash_table->table_size;
    old_prime_index = hash_table->prime_index;

    /* Allocate a new, larger table */

    ++hash_table->prime_index;

    if (!hash_table_allocate_table(hash_table))
    {

        /* Failed to allocate the new table */

        hash_table->table = old_table;
        hash_table->table_size = old_table_size;
        hash_table->prime_index = old_prime_index;

        return 0;
    }

    /* Link all entries from all chains into the new table */

    for (i = 0; i < old_table_size; ++i)
    {
        rover = old_table[i];

        while (rover != NULL)
        {
            next = rover->next;

            /* Fetch rover HashTablePair */

            pair = &(rover->pair);

            /* Find the index into the new table */

            index = pair->key % hash_table->table_size;

            /* Link this entry into the chain */

            rover->next = hash_table->table[index];
            hash_table->table[index] = rover;

            /* Advance to next in the chain */

            rover = next;
        }
    }

    /* Free the old table */

    free(old_table);

    return 1;
}

int hash_table_insert(HashTable *hash_table, int key, int value)
{
    HashTableEntry *rover;
    HashTablePair *pair;
    HashTableEntry *newentry;
    unsigned int index;

    /* If there are too many items in the table with respect to the table
     * size, the number of hash collisions increases and performance
     * decreases. Enlarge the table size to prevent this happening */

    if ((hash_table->entries * 3) / hash_table->table_size > 0)
    {

        /* Table is more than 1/3 full */

        if (!hash_table_enlarge(hash_table))
        {

            /* Failed to enlarge the table */

            return 0;
        }
    }

    /* Generate the hash of the key and hence the index into the table */

    index = key % hash_table->table_size;

    /* Traverse the chain at this location and look for an existing
     * entry with the same key */

    rover = hash_table->table[index];

    while (rover != NULL)
    {

        /* Fetch rover's HashTablePair entry */

        pair = &(rover->pair);

        if (pair->key == key)
        {
            /* Same key: add to linked list new item */
            pair->key = key;
            return list_insert(pair->values, value);
        }

        rover = rover->next;
    }

    /* Not in the hash table yet.  Create a new entry */

    newentry = (HashTableEntry *)malloc(sizeof(HashTableEntry));
    if (newentry == NULL)
        return 0;

    newentry->pair.values = list_new();
    if (!newentry->pair.values)
    {
        free(newentry);
        return 0;
    }

    newentry->pair.key = key;
    if (!list_insert(newentry->pair.values, value)) return 0;

    /* Link into the list */

    newentry->next = hash_table->table[index];
    hash_table->table[index] = newentry;

    /* Maintain the count of the number of entries */

    ++hash_table->entries;

    /* Added successfully */

    return 1;
}

List *hash_table_lookup(HashTable *hash_table, int key)
{
    HashTableEntry *rover;
    HashTablePair *pair;
    unsigned int index;

    /* Generate the hash of the key and hence the index into the table */

    index = key % hash_table->table_size;

    /* Walk the chain at this index until the corresponding entry is
     * found */

    rover = hash_table->table[index];

    while (rover != NULL)
    {
        pair = &(rover->pair);

        if (key == pair->key)
            return pair->values;

        rover = rover->next;
    }

    /* Not found */

    return HASH_TABLE_NULL;
}

unsigned int hash_table_num_entries(HashTable *hash_table)
{
    return hash_table->entries;
}

void hash_table_iterate(HashTable *hash_table, HashTableIterator *iterator)
{
    unsigned int chain;

    iterator->hash_table = hash_table;

    /* Default value of next if no entries are found. */

    iterator->next_entry = NULL;

    /* Find the first entry */

    for (chain = 0; chain < hash_table->table_size; ++chain)
    {

        if (hash_table->table[chain] != NULL)
        {
            iterator->next_entry = hash_table->table[chain];
            iterator->next_chain = chain;
            break;
        }
    }
}

int hash_table_iter_has_more(HashTableIterator *iterator)
{
    return iterator->next_entry != NULL;
}

HashTablePair hash_table_iter_next(HashTableIterator *iterator)
{
    HashTableEntry *current_entry;
    HashTable *hash_table;
    HashTablePair pair = {NULL, NULL};
    unsigned int chain;

    hash_table = iterator->hash_table;

    if (iterator->next_entry == NULL)
    {
        return pair;
    }

    /* Result is immediately available */

    current_entry = iterator->next_entry;
    pair = current_entry->pair;

    /* Find the next entry */

    if (current_entry->next != NULL)
    {

        /* Next entry in current chain */

        iterator->next_entry = current_entry->next;
    }
    else
    {

        /* None left in this chain, so advance to the next chain */

        chain = iterator->next_chain + 1;

        /* Default value if no next chain found */

        iterator->next_entry = NULL;

        while (chain < hash_table->table_size)
        {

            /* Is there anything in this chain? */

            if (hash_table->table[chain] != NULL)
            {
                iterator->next_entry = hash_table->table[chain];
                break;
            }

            /* Try the next chain */

            ++chain;
        }

        iterator->next_chain = chain;
    }

    return pair;
}