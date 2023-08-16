// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

#define MAX_ENTRIES 1024

#define DECLARE_GENERIC_TEST_MAP(TYPE)                                                             \
    struct                                                                                         \
    {                                                                                              \
        __uint(type, BPF_MAP_TYPE_##TYPE);                                                         \
        __uint(max_entries, MAX_ENTRIES);                                                          \
        __type(key, int);                                                                          \
        __type(value, int);                                                                        \
    } TYPE##_map SEC(".maps");                                                                     \
    struct                                                                                         \
    {                                                                                              \
        __uint(type, BPF_MAP_TYPE_ARRAY);                                                          \
        __uint(max_entries, 1);                                                                    \
        __type(key, int);                                                                          \
        __type(value, int);                                                                        \
    } TYPE##_map_init SEC(".maps");                                                                \
    __attribute__((section("xdp/prep_" #TYPE))) int prepare_##TYPE##_state_generic_read(void* ctx) \
    {                                                                                              \
        int key = 0;                                                                               \
        int* value = bpf_map_lookup_elem(&TYPE##_map_init, &key);                                  \
        if (value && *value < MAX_ENTRIES) {                                                       \
            int i = *value;                                                                        \
            bpf_map_update_elem(&TYPE##_map, &i, &i, BPF_ANY);                                     \
            *value += 1;                                                                           \
        }                                                                                          \
        return 0;                                                                                  \
    }                                                                                              \
    __attribute__((section("xdp/read_" #TYPE))) int read_##TYPE(void* ctx)                         \
    {                                                                                              \
        int key = bpf_get_prandom_u32() % MAX_ENTRIES;                                             \
        int* value = bpf_map_lookup_elem(&TYPE##_map, &key);                                       \
        if (value) {                                                                               \
            return 0;                                                                              \
        }                                                                                          \
        return 1;                                                                                  \
    }                                                                                              \
    __attribute__((section("xdp/_update" #TYPE))) int update_##TYPE(void* ctx)                     \
    {                                                                                              \
        int key = bpf_get_prandom_u32() % MAX_ENTRIES;                                             \
        return bpf_map_update_elem(&TYPE##_map, &key, &key, BPF_ANY);                              \
    }

DECLARE_GENERIC_TEST_MAP(HASH)
DECLARE_GENERIC_TEST_MAP(ARRAY)
DECLARE_GENERIC_TEST_MAP(PERCPU_HASH)
DECLARE_GENERIC_TEST_MAP(PERCPU_ARRAY)
DECLARE_GENERIC_TEST_MAP(LRU_HASH)
