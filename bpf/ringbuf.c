// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

#if !defined(RB_SIZE)
#define RB_SIZE (128 * 1024)
#endif

#if !defined(RECORD_SIZE)
#define RECORD_SIZE 128
#endif

struct
{
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, RB_SIZE);
} rb_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __type(key, int);
    __uint(max_entries, 1);
    __uint(value_size, RECORD_SIZE);
} buf_map SEC(".maps");

SEC("sockops/bpf_ringbuf_output") int output(void* ctx)
{
    int key = 0;
    void* msg = bpf_map_lookup_elem(&buf_map, &key);
    if (!msg) {
        return 1;
    }
    if (bpf_ringbuf_output(&rb_map, msg, RECORD_SIZE, 0) < 0) {
        return 1;
    } else {
        return 0;
    }
}
