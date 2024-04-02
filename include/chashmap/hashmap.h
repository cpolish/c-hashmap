/**
 * Copyright (c) 2023, Chris Polak
 * All rights reserved.
 * 
 * This work is licensed under the Mozilla Public License Version 2.0, more details can 
 * be found in the LICENSE file.
 * 
 * 
 * A header-only library utilising macro tricks to implement hashmaps in C.
 * 
 * AUTHOR: Chris Polak
 * VERSION: 0.1
 */
#ifndef CPOLISH_CHASHMAP_HASHMAP_H
#define CPOLISH_CHASHMAP_HASHMAP_H

/** @file */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "./_hashmap.h"


/**
 * @brief Define a struct for a hashmap entry.
 * 
 * @param entry_t_name the type name for the struct that will be generated to match a 
 * hashmap entry.
 * @param k the type of the hashmap's key.
 * @param v the type of the hashmap's value.
 */
#define HASHMAP_ENTRY(entry_t_name, k, v)                                                \
    struct entry_t_name {                                                                \
        k key;                                                                           \
        v value;                                                                         \
    }

/**
 * @brief Defines a hashmap data type with a struct.
 *
 * This macro also defines some private helper structs which aid in the function of the 
 * hashmap.
 * 
 * Note that the `HASHMAP_ENTRY` type should be defined when using this macro, as is uses 
 * the type name of the hashmap entry when generating the hashmap struct.
 *
 * @param t_name the type name for the hashmap.
 * @param entry_t_name the type name for the hashmap entry struct previously defined in 
 * `HASHMAP_ENTRY`.
 * @param k_type the type of the keys for the hashmap. Required to generate an 
 * appropriate function pointer for the hashmap's hash function.
 */
#define HASHMAP(t_name, entry_t_name, k_type)                                            \
    struct _t_##entry_t_name {                                                           \
        struct entry_t_name _entry;                                                      \
        struct _t_##entry_t_name *_next_entry;                                           \
        struct _ll_node_##t_name *_ll_node;                                              \
    };                                                                                   \
                                                                                         \
    struct _ll_node_##t_name {                                                           \
        struct _t_##entry_t_name *_table_entry;                                          \
        struct _ll_node_##t_name *_previous;                                             \
        struct _ll_node_##t_name *_next;                                                 \
    };                                                                                   \
                                                                                         \
    struct t_name {                                                                      \
        size_t _table_size;                                                              \
        size_t _num_entries;                                                             \
        struct {                                                                         \
            struct _ll_node_##t_name *_head;                                             \
            struct _ll_node_##t_name *_tail;                                             \
        } _entry_ll;                                                                     \
        struct _t_##entry_t_name **_entry_table;                                         \
        size_t (*hash_func)(size_t, k_type);                                             \
    }

/**
 * @brief Initialise a hashmap to an empty, new state.
 * 
 * @param m a pointer to the hashmap struct to initialise.
 * 
 * @return '0' for a successful initialisation, and '-1' if an error occurred during 
 * initialisation, such as allocating memory.
 */
#define HASHMAP_INIT(m) ({                                                               \
    int _return_val = 0;                                                                 \
                                                                                         \
    (m)->_table_size = _DEFAULT_TABLE_SIZE;                                              \
    (m)->_num_entries = 0;                                                               \
    (m)->_entry_ll._head = NULL;                                                         \
    (m)->_entry_ll._tail = NULL;                                                         \
                                                                                         \
    typedef _HASHMAP_KEY_TYPE(m) _key_type;                                              \
    if (_IS_INTEGER(_key_type)) {                                                        \
        (m)->hash_func = _hash_long_long;                                                \
    } else if (_IS_FLOATING_POINT(_key_type)) {                                          \
        (m)->hash_func = _hash_double;                                                   \
    } else {                                                                             \
        (m)->hash_func = _hash_ptr;                                                      \
    }                                                                                    \
                                                                                         \
    (m)->_entry_table = calloc((m)->_table_size, sizeof(_HASHMAP_ENTRY_PTR_TYPE(m)));    \
    if ((m)->_entry_table == NULL) {                                                     \
        _return_val = -1;                                                                \
    }                                                                                    \
                                                                                         \
    _return_val;                                                                         \
})

/**
 * @brief Place a value `v` which corresponds to the provided key `k` in the provided 
 * hashmap.
 * 
 * If `k` already exists in the hashmap, the existing value is replaced by that provided 
 * by the parameter `v`.
 * 
 * @param m a pointer to the hashmap struct to put a value into.
 * @param k the key which maps to the value provided by `v`.
 * @param v the value which is mapped from the key `k`.
 * 
 * @return '0' for a successful insertion of the key-value pair in the hashmap, or '-1' 
 * if an error occurred whilst placing the value, such as when allocating memory for the 
 * key-value entry, or resizing the hashmap's hash table.
 */
#define HASHMAP_PUT(m, k, v) ({                                                          \
    double _load_factor = (double) (m)->_num_entries / (m)->_table_size;                 \
                                                                                         \
    int _return_val = -1;                                                                \
    if (_load_factor >= _LOAD_FACTOR_LIM) {                                              \
        int _increase_size_res = _HASHMAP_INCREASE_SIZE((m));                            \
        if (_increase_size_res == 0) {                                                   \
            __auto_type _table_put_res = _HASHMAP_TABLE_PUT((m), (k), (v));              \
            int _ll_put_res = 0;                                                         \
            if (_table_put_res != NULL) {                                                \
                _ll_put_res = _HASHMAP_LL_PUT((m), _table_put_res);                      \
            }                                                                            \
                                                                                         \
            if (_ll_put_res == 0) {                                                      \
                _return_val = 0;                                                         \
            }                                                                            \
        }                                                                                \
    } else {                                                                             \
        __auto_type _table_put_res = _HASHMAP_TABLE_PUT((m), (k), (v));                  \
        int _ll_put_res = 0;                                                             \
        if (_table_put_res != NULL) {                                                    \
            _ll_put_res = _HASHMAP_LL_PUT((m), _table_put_res);                          \
        }                                                                                \
                                                                                         \
        if (_ll_put_res == 0) {                                                          \
            _return_val = 0;                                                             \
        }                                                                                \
    }                                                                                    \
                                                                                         \
    (m)->_num_entries++;                                                                 \
    _return_val;                                                                         \
})

/**
 * @brief Attempt to obtain a value mapped by the key `k` in the provided hashmap.
 * 
 * @param result a pointer to store the value mapped by the key `k` in the hashmap, if 
 * this key-value entry exists.
 * @param m a pointer to the hashmap struct to attempt to get the value mapped by `k` 
 * from.
 * @param k a key to obtain its corresponding mapped value from the hashmap `m`, if such 
 * a key-value entry exists.
 * 
 * @return A `bool` value: `true` if the key `k` existed in the hashmap, and a value was 
 * successfully obtained and stored in `result`, `false` otherwise.
 */
#define HASHMAP_GET(result, m, k) ({                                                     \
    size_t _hash_val = (m)->hash_func((m)->_table_size, (k));                            \
    __auto_type _table_cell = (m)->_entry_table[_hash_val];                              \
    typeof(*result) _val_search_res;                                                     \
    bool _found_key = false;                                                             \
    while (_table_cell != NULL) {                                                        \
        if (_table_cell->_entry.key == k) {                                              \
            _val_search_res = _table_cell->_entry.value;                                 \
            _found_key = true;                                                           \
            break;                                                                       \
        }                                                                                \
                                                                                         \
        _table_cell = _table_cell->_next_entry;                                          \
    }                                                                                    \
                                                                                         \
    bool _return_val = false;                                                            \
    if (_found_key) {                                                                    \
        *(result) = _val_search_res;                                                     \
        _return_val = true;                                                              \
    }                                                                                    \
                                                                                         \
    _return_val;                                                                         \
})

/**
 * @brief Free all allocated memory for the provided hashmap. This macro should be called 
 * after a user has finished using a hashmap.
 * 
 * @param m a pointer to the hashmap struct to free memory from.
 */
#define HASHMAP_FREE(m) ({                                                               \
    _HASHMAP_TABLE_FREE((m));                                                            \
    _HASHMAP_LL_FREE((m));                                                               \
})

#endif
