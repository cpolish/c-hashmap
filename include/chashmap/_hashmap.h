/**
 * Copyright (c) 2023, Chris Polak
 * All rights reserved.
 * 
 * This work is licensed under the Mozilla Public License Version 2.0, more details can 
 * be found in the LICENSE file.
 * 
 * 
 * Internal helper functions to assist with the general hashmap header library.
 * 
 * AUTHOR: Chris Polak
 * VERSION: 0.1
 */
#ifndef CPOLISH_CHASHMAP__HASHMAP_H
#define CPOLISH_CHASHMAP__HASHMAP_H

/** @file */

#include <stdbool.h>
#include <stddef.h>


/**
 * @brief The default hash table size for a newly-initialised hash table.
 */
#define _DEFAULT_TABLE_SIZE 15

/**
 * @brief The default load factor limit used for the hashmap. Based off the load factor 
 * limit used in Java's `HashMap` implementation.
 */
#define _LOAD_FACTOR_LIM 0.75

/**
 * Helper macros used to check if a data type is an integer or a floating-point data type.
 * 
 * Method credit to "phicr" from the following StackOverflow answer: 
 * https://stackoverflow.com/a/75276955.
 */

#define _IS_INTEGER(t) \
        _Generic(*((t *) 0), \
                 _Bool: true, \
                 unsigned char: true, \
                 char: true, \
                 unsigned short: true, \
                 short: true, \
                 unsigned int: true, \
                 int: true, \
                 unsigned long: true, \
                 long: true, \
                 unsigned long long: true, \
                 long long: true, \
                 default: false) \

#define _IS_FLOATING_POINT(t) \
        _Generic(*((t *) 0), \
                 float: true, \
                 double: true, \
                 default: false)

/**
 * Helper macros to obtain a specific data type for hashmap operations, based on the 
 * field of the hashmap struct.
 */

#define _HASHMAP_ENTRY_TYPE(m) typeof(**((m)->_entry_table))
#define _HASHMAP_ENTRY_PTR_TYPE(m) typeof(*((m)->_entry_table))
#define _HASHMAP_KEY_TYPE(m) typeof(((*((m)->_entry_table))->_entry).key)
#define _HASHMAP_LL_NODE_TYPE(m) typeof(*((m)->_entry_ll._head))


/**
 * Inline hash functions for the built-in C data types.
 */

static inline size_t _hash_long_long(size_t table_size, long long val) {
    return val % table_size;
}

static inline size_t _hash_double(size_t table_size, double val) {
    union {
        double d;
        size_t s;
    } val_bits = {.d = val};

    return val_bits.s % table_size;
}

static inline size_t _hash_ptr(size_t table_size, void *ptr) {
    return (((size_t) ptr * 6) % (table_size * 2)) % table_size;
}


/**
 * Helper macros to assist with macros defined in `hashmap.h`.
 */

/**
 * @brief Frees the linked list representing all entries within a hashmap.
 * 
 * @param m a pointer to the hashmap struct to free linked list entries.
 */
#define _HASHMAP_LL_FREE(m) ({                                                           \
    __auto_type _ll_node_cursor = m->_entry_ll._head;                                    \
    while (_ll_node_cursor != NULL) {                                                    \
        __auto_type _node_to_free = _ll_node_cursor;                                     \
        _ll_node_cursor = _ll_node_cursor->_next;                                        \
                                                                                         \
        free(_node_to_free);                                                             \
    }                                                                                    \
})

/**
 * @brief Frees the hashmap's hash table, including all associated entries.
 * 
 * @param m a pointer to the hashmap struct to free hash table entries.
 */
#define _HASHMAP_TABLE_FREE(m) ({                                                        \
    /* Free hash table entries */                                                        \
    __auto_type _ll_node_cursor = m->_entry_ll._head;                                    \
    while (_ll_node_cursor != NULL) {                                                    \
        __auto_type _entry_ptr = _ll_node_cursor->_table_entry;                          \
        free(_entry_ptr);                                                                \
                                                                                         \
        _ll_node_cursor = _ll_node_cursor->_next;                                        \
    }                                                                                    \
                                                                                         \
    /* Free hash table */                                                                \
    free((m)->_entry_table);                                                             \
})

/**
 * @brief Remove a node from the linked list of all entries in the hashmap.
 * 
 * Code sourced from example Wikipedia algorithm: 
 * https://en.wikipedia.org/wiki/Doubly_linked_list#Removing_a_node.
 * 
 * @param m a pointer to the hashmap struct.
 * @param target_n_pointer the pointer to the target node to remove from the linked list.
 */
#define _HASHMAP_LL_REMOVE(m, target_n_ptr) ({                                           \
    if (target_n_ptr->_previous == NULL) {                                               \
        m->_entry_ll._head = target_n_ptr->_next;                                        \
    } else {                                                                             \
        target_n_ptr->_previous->_next = target_n_ptr->_next;                            \
    }                                                                                    \
                                                                                         \
    if (target_n_ptr->_next == NULL) {                                                   \
        m->_entry_ll._tail = target_n_ptr->_previous;                                    \
    } else {                                                                             \
        target_n_ptr->_next->_previous = target_n_ptr->_previous;                        \
    }                                                                                    \
                                                                                         \
    free((target_n_ptr));                                                                \
})

/**
 * @brief Insert a new node after a specified node in the hashmap's entry linked list.
 * 
 * Code sourced from example Wikipedia algorithm: 
 * https://en.wikipedia.org/wiki/Doubly_linked_list#Inserting_a_node.
 * 
 * @param m a pointer to the hashmap struct.
 * @param node the pointer to the node in which to insert a new entry after.
 * @param new_node the pointer to the new node to insert following `node`.
 */
#define _HASHMAP_LL_ADD_AFTER(m, node, new_node) ({                                      \
    new_node->_previous = node;                                                          \
                                                                                         \
    if (node->_next == NULL) {                                                           \
        new_node->_next = NULL;                                                          \
        m->_entry_ll._tail = new_node;                                                   \
    } else {                                                                             \
        new_node->_next = node->_next;                                                   \
        node->_next->_previous = new_node;                                               \
    }                                                                                    \
                                                                                         \
    node->_next = new_node;                                                              \
})

/**
 * @brief Insert a new node before a specified node in the hashmap's entry linked list.
 * 
 * Code sourced from example Wikipedia algorithm: 
 * https://en.wikipedia.org/wiki/Doubly_linked_list#Inserting_a_node.
 * 
 * @param m a pointer to the hashmap struct.
 * @param node the pointer to the node in which to insert a new entry before.
 * @param new_node the pointer to the new node to insert prior to `node`.
 */
#define _HASHMAP_LL_ADD_BEFORE(m, node, new_node) ({                                     \
    new_node->_next = node;                                                              \
                                                                                         \
    if (node->_previous == NULL) {                                                       \
        new_node->_previous = NULL;                                                      \
        m->_entry_ll._head = new_node;                                                   \
    } else {                                                                             \
        new_node->_previous = node->_previous;                                           \
        node->_previous->_next = new_node;                                               \
    }                                                                                    \
                                                                                         \
    node->_previous = new_node;                                                          \
})

/**
 * @brief Insert a new node at the head of the hashmap's entry linked list.
 * 
 * Code sourced from example Wikipedia algorithm: 
 * https://en.wikipedia.org/wiki/Doubly_linked_list#Inserting_a_node.
 * 
 * @param m a pointer to the hashmap struct.
 * @param n the pointer to the new node to insert at the beginning (head) of the 
 * hashmap's entry linked list.
 */
#define _HASHMAP_LL_ADD_HEAD(m, n) ({                                                    \
    if (m->_entry_ll._head == NULL) {                                                    \
        m->_entry_ll._head = n;                                                          \
        m->_entry_ll._tail = n;                                                          \
                                                                                         \
        n->_previous = NULL;                                                             \
        n->_next = NULL;                                                                 \
    } else {                                                                             \
        __auto_type _head_ptr = m->_entry_ll._head;                                      \
        _HASHMAP_LL_ADD_BEFORE(m, _head_ptr, n);                                         \
    }                                                                                    \
})

/**
 * @brief Insert a new node at the tail of the hashmap's entry linked list.
 * 
 * Code sourced from example Wikipedia algorithm: 
 * https://en.wikipedia.org/wiki/Doubly_linked_list#Inserting_a_node.
 * 
 * @param m a pointer to the hashmap struct.
 * @param n the pointer to the new node to insert at the end (tail) of the hashmap's 
 * entry linked list.
 */
#define _HASHMAP_LL_ADD_TAIL(m, n) ({                                                    \
    if (m->_entry_ll._tail == NULL) {                                                    \
        _HASHMAP_LL_ADD_HEAD(m, n);                                                      \
    } else {                                                                             \
        __auto_type _tail_ptr = m->_entry_ll._tail;                                      \
        _HASHMAP_LL_ADD_AFTER(m, _tail_ptr, n);                                          \
    }                                                                                    \
})

/**
 * @brief Allocate and insert a new linked list node at the end of the hashmap's entry 
 * linked list, pointing to the table's entry node.
 * 
 * @param m a pointer to the hashmap struct.
 * @param entry_ptr the pointer to a entry in the hashmap's entry table, which will be 
 * the value of the entry linked list node.
 * 
 * @return '0' for a successful insertion at the end of the hashmap's entry linked list, 
 * '-1' if a failure occurred due to issues such as a memory allocation.
 */
#define _HASHMAP_LL_PUT(m, entry_ptr) ({                                                 \
    typedef _HASHMAP_LL_NODE_TYPE(m) _ll_node_type;                                      \
                                                                                         \
    _ll_node_type *_new_entry_ll_node = calloc(1, sizeof(_ll_node_type));                \
                                                                                         \
    int _return_val = -1;                                                                \
    if (_new_entry_ll_node != NULL) {                                                    \
        _new_entry_ll_node->_table_entry = entry_ptr;                                    \
                                                                                         \
        entry_ptr->_ll_node = _new_entry_ll_node;                                        \
                                                                                         \
        _HASHMAP_LL_ADD_TAIL(m, _new_entry_ll_node);                                     \
        _return_val = 0;                                                                 \
    }                                                                                    \
                                                                                         \
    _return_val;                                                                         \
})

/**
 * @brief Search and attempt to find a hash table entry for the provided key `k`.
 * 
 * @param m a pointer to the hashmap struct.
 * @param k the key value in which the hash table will be searched to find a matching 
 * key-value entry.
 * 
 * @return A pointer to a pointer which refers to the key-value entry in the hash table, 
 * where the key-value entry pointer may be `NULL` if no matching key-value entry exists.
 */
#define _HASHMAP_FIND_ENTRY_RECORD(m, k) ({ \
    size_t _hash_val = m->hash_func(m->_table_size, k);                                  \
                                                                                         \
    /* Search for matching key record */                                                 \
    __auto_type _entry_cursor = &(m->_entry_table[_hash_val]);                           \
    while (*_entry_cursor != NULL) {                                                     \
        if ((*_entry_cursor)->_entry.key == k) {                                         \
            break;                                                                       \
        }                                                                                \
                                                                                         \
        _entry_cursor = &((*_entry_cursor)->_next_entry);                                \
    }                                                                                    \
                                                                                         \
    /* Return entry cursor */                                                            \
    _entry_cursor;                                                                       \
})

/**
 * @brief Place or update a key-value entry in the provided hashmap
 * 
 * @param m a pointer to the hashmap struct.
 * @param k the key in which to place or update a key-value entry with value `v`.
 * @param v the value which will be mapped to the key `k` in a new or existing key-value 
 * entry.
 * 
 * @return A pointer to the key-value entry in the hash table if a new entry was 
 * allocated, otherwise, `NULL` if either:
 * 
 * - the key-value entry already existed in the hash table, and the value was simply 
 *   updated.
 * - an error occurred when attempting to allocate memory for a new entry.
 */
#define _HASHMAP_TABLE_PUT(m, k, v) ({                                                   \
    typedef _HASHMAP_ENTRY_TYPE(m) _entry_type;                                          \
                                                                                         \
    __auto_type _entry_record = _HASHMAP_FIND_ENTRY_RECORD(m, k);                        \
                                                                                         \
    _entry_type *_return_val = NULL;                                                     \
    if (*_entry_record != NULL) {                                                        \
        /* Record with key exists */                                                     \
        (*_entry_record)->_entry.value = v;                                              \
    } else {                                                                             \
        /* Record doesn't exist - allocate new memory */                                 \
        _entry_type *_new_entry = calloc(1, sizeof(_entry_type));                        \
        if (_new_entry != NULL) {                                                        \
            _new_entry->_entry.key = k;                                                  \
            _new_entry->_entry.value = v;                                                \
            _new_entry->_next_entry = NULL;                                              \
                                                                                         \
            /* Place in the linked list */                                               \
            size_t _hash_val = m->hash_func(m->_table_size, k);                          \
            __auto_type _table_entry_cell = &(m->_entry_table[_hash_val]);               \
            if (*_table_entry_cell == NULL) {                                            \
                /* Cell has no entries, so we make a new one here */                     \
                *_table_entry_cell = _new_entry;                                         \
            } else {                                                                     \
                /* Cell has entries - add to the end */                                  \
                __auto_type _table_entry_cursor = _table_entry_cell;                     \
                while ((*_table_entry_cursor)->_next_entry != NULL) {                    \
                    _table_entry_cursor = &((*_table_entry_cursor)->_next_entry);        \
                }                                                                        \
                                                                                         \
                /* Record as next entry */                                               \
                (*_table_entry_cursor)->_next_entry = _new_entry;                        \
            }                                                                            \
                                                                                         \
            /* Return pointer back */                                                    \
            _return_val = _new_entry;                                                    \
        }                                                                                \
    }                                                                                    \
                                                                                         \
    _return_val;                                                                         \
})

/**
 * @brief Rehash a hashmap's hash table after it has been resized and a new table 
 * allocated.
 * 
 * @param m a pointer to the hashmap struct.
 * 
 * @return '0' if all entries were successfully rehashed to a new hash table, otherwise 
 * '-1' if a failure occurred due to errors such as in memory allocation.
 */
#define _HASHMAP_REHASH_TABLE(m) ({                                                      \
    typedef _HASHMAP_ENTRY_TYPE(m) _entry_type;                                          \
                                                                                         \
    int _return_val = 0;                                                                 \
                                                                                         \
    __auto_type _ll_node_cursor = m->_entry_ll._head;                                    \
    while (_ll_node_cursor != NULL) {                                                    \
        __auto_type _table_entry = _ll_node_cursor->_table_entry;                        \
        __auto_type _entry_key = _table_entry->_entry.key;                               \
        __auto_type _entry_value = _table_entry->_entry.value;                           \
                                                                                         \
        __auto_type _rehashed_table_entry = _HASHMAP_TABLE_PUT(m, _entry_key,            \
                                                               _entry_value);            \
        if (_rehashed_table_entry == NULL) {                                             \
            _return_val = -1;                                                            \
            break;                                                                       \
        }                                                                                \
                                                                                         \
        /* Change node details */                                                        \
        _ll_node_cursor->_table_entry = _rehashed_table_entry;                           \
                                                                                         \
        /* Free old entry */                                                             \
        free(_table_entry);                                                              \
                                                                                         \
        _ll_node_cursor = _ll_node_cursor->_next;                                        \
    }                                                                                    \
                                                                                         \
    _return_val;                                                                         \
})

/**
 * @brief Increase the size of a hashmap's hash table by allocating a new table in memory 
 * double the size of the existing hash table.
 * 
 * @param m a pointer to the hashmap struct.
 * 
 * @return '0' for a successful allocation of a larger hash table, otherwise '-1' if an 
 * error occurred during memory allocation.
 */
#define _HASHMAP_INCREASE_SIZE(m) ({                                                     \
    size_t _old_table_size = m->_table_size;                                             \
    __auto_type _old_table = m->_entry_table;                                            \
                                                                                         \
    size_t _new_table_size = _old_table_size * 2;                                        \
    typeof(m->_entry_table) _new_table_ptr =                                             \
            calloc(_new_table_size, sizeof(_HASHMAP_ENTRY_PTR_TYPE(m)));                 \
                                                                                         \
    int _return_val = -1;                                                                \
    if (_new_table_ptr != NULL) {                                                        \
        m->_table_size = _new_table_size;                                                \
        m->_entry_table = _new_table_ptr;                                                \
                                                                                         \
        _return_val = _HASHMAP_REHASH_TABLE(m);                                          \
                                                                                         \
        /* Free old table */                                                             \
        free(_old_table);                                                                \
    }                                                                                    \
                                                                                         \
    _return_val;                                                                         \
})

#endif
