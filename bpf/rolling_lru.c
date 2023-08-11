// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

#define MAX_ENTRIES 8192
#define KEY_RANGE (MAX_ENTRIES / 10) // 10% of MAX_ENTRIES

// This test measures the performance of the LRU hash with a rolling key set.
// Searches are performed in the LRU map using keys in the range [lru_key_base, lru_key_base + lru_key_range).
// If the key is found in the map, it is updated with 0.
// If the key is not found in the map, it is added to the map with value 0.
// lru_key_base is incremented by 1 on every 16th iteration. This is done to simulate a rolling key set.

struct
{
    __uint(type, BPF_MAP_TYPE_LRU_HASH);
    __uint(max_entries, MAX_ENTRIES);
    __type(key, int);
    __type(value, int);
} rolling_lru_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, int);
    __type(value, int);
} rolling_lru_map_init SEC(".maps");

// lru_key_base is incremented by 1 on every 16th iteration.
// Value stored in the array is the current value of lru_key_base shifted right by 4 bits.
struct
{
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __uint(max_entries, MAX_ENTRIES);
    __type(key, int);
    __type(value, int);
} lru_key_base SEC(".maps");

// Populate the LRU map with keys in the range [lru_key_base, lru_key_base + lru_key_range).
__attribute__((section("xdp"))) int
prepare_rolling_lru_state(void* ctx)
{
    int key = 0;
    int* value = bpf_map_lookup_elem(&rolling_lru_map_init, &key);
    if (value && *value < MAX_ENTRIES) {
        bpf_map_update_elem(&rolling_lru_map, value, value, BPF_ANY);
        *value += 1;
    }
    return 0;
}

// Search for a random key in the LRU map in the range [lru_key_base, lru_key_base + lru_key_range).
// If found in the map, update the value to 0.
// If not found in the map, add the key to the map with value 0.
// Increment lru_key_base by 1 on every 10th iteration on CPU 0.
__attribute__((section("xdp"))) int
read_or_update_rolling_lru(void* ctx)
{
    int key = bpf_get_prandom_u32() % KEY_RANGE;
    int zero = 0;
    int* key_base = bpf_map_lookup_elem(&lru_key_base, &zero);
    if (!key_base) {
        return 1;
    }

    *key_base += 1;
    key += (*key_base) >> 4;

    // Update the key in the map if it exists.
    int* value = bpf_map_lookup_elem(&rolling_lru_map, &key);
    if (value) {
        *value = 0;
    }
    // Otherwise, add the key to the map.
    else {
        bpf_map_update_elem(&rolling_lru_map, &key, &zero, BPF_ANY);
    }
    return 0;
}