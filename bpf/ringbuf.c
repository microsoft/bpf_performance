// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

#if !defined(MAX_RECORDS)
#define MAX_RECORDS 1024
#endif

#if !defined(RECORD_SIZE)
#define RECORD_SIZE 128
#endif

struct
{
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, MAX_RECORDS* RECORD_SIZE);
} map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, int);
    __uint(value_size, RECORD_SIZE);
} map2 SEC(".maps");

SEC("sockops/bpf_ringbuf_output") int output(void* ctx)
{
    void* msg;
    int key = 0;
    msg = bpf_map_lookup_elem(&map2, &key);
    if (msg == NULL) {
        return 1;
    }
    if (bpf_ringbuf_output(&map, msg, RECORD_SIZE, 0) < 0) {
        return 1;
    } else {
        return 0;
    }
}
