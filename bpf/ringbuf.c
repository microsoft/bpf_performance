// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

struct
{
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 128 * 1024);
} map SEC(".maps");

SEC("xdp/bpf_ringbuf_output") int output(void* ctx)
{
    char msg[] = "Hello, World!";
    if (bpf_ringbuf_output(&map, msg, sizeof(msg), 0) < 0) {
        return 1;
    } else {
        return 0;
    }
}
