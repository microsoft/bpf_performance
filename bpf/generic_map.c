// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

#if !defined(MAX_ENTRIES)
#define MAX_ENTRIES 1024
#endif

#if !defined(TYPE)
#define TYPE BPF_MAP_TYPE_HASH
#endif

struct
{
    __uint(type, TYPE);
    __uint(max_entries, MAX_ENTRIES);
    __type(key, int);
    __type(value, int);
} map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, int);
    __type(value, int);
} map_init SEC(".maps");

SEC("xdp/prepare") int prepare(void* ctx)
{
    int key = 0;
    int* value = bpf_map_lookup_elem(&map_init, &key);
    if (value && *value < MAX_ENTRIES) {
        int i = *value;
        bpf_map_update_elem(&map, &i, &i, BPF_ANY);
        *value += 1;
    }
    return 0;
}

SEC("xdp/read") int read(void* ctx)
{
    int key = bpf_get_prandom_u32() % MAX_ENTRIES;
    int* value = bpf_map_lookup_elem(&map, &key);
    if (value) {
        return 0;
    }
    return 1;
}

SEC("xdp/update") int update(void* ctx)
{
    int key = bpf_get_prandom_u32() % MAX_ENTRIES;
    return bpf_map_update_elem(&map, &key, &key, BPF_ANY);
}

SEC("xdp/replace") int replace(void* ctx)
{
    int key = bpf_get_prandom_u32() % MAX_ENTRIES;
    (void)bpf_map_delete_elem(&map, &key);
    (void)bpf_map_update_elem(&map, &key, &key, BPF_ANY);
    return 0;
}
