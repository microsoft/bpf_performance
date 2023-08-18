// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

#define MAX_ENTRIES 8192

struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, unsigned int);
    __type(value, unsigned int);
    __uint(max_entries, MAX_ENTRIES);
} inner_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_ARRAY_OF_MAPS);
    __type(key, unsigned int);
    __type(value, unsigned int);
    __uint(max_entries, 1);
    __array(values, inner_map);
} outer_map SEC(".maps") = {
    .values = {&inner_map},
};

struct
{
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __type(key, unsigned int);
    __type(value, unsigned int);
    __uint(max_entries, 1);
} outer_map_init SEC(".maps");

SEC("xdp/prep_map_in_map") int prepare_map_in_map(void* ctx)
{
    int key = 0;
    int* value = bpf_map_lookup_elem(&outer_map_init, &key);
    if (value && *value < MAX_ENTRIES) {
        int i = *value;
        bpf_map_update_elem(&inner_map, &i, &i, BPF_ANY);
        *value += 1;
    }
    return 0;
}

SEC("xdp/read_map_in_map") int read_map_in_map(void* ctx)
{
    int outer_key = 0;
    int key = bpf_get_prandom_u32() % MAX_ENTRIES;
    void* inner_map = bpf_map_lookup_elem(&outer_map, &outer_key);
    if (!inner_map) {
        return 1;
    }
    int* value = bpf_map_lookup_elem(inner_map, &key);
    if (value) {
        return 0;
    }

    return 1;
}

SEC("xdp/write_map_in_map") int write_map_in_map(void* ctx)
{
    int outer_key = 0;
    int key = bpf_get_prandom_u32() % MAX_ENTRIES;
    void* inner_map = bpf_map_lookup_elem(&outer_map, &outer_key);
    if (!inner_map) {
        return 1;
    }
    return bpf_map_update_elem(inner_map, &key, &key, BPF_ANY);
}
