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
    __uint(max_entries, (MAX_RECORDS * RECORD_SIZE));
} map SEC(".maps");

SEC("sockops/bpf_ringbuf_output") int output(void* ctx)
{
    char msg[RECORD_SIZE] = {0};
    if (bpf_ringbuf_output(&map, msg, sizeof(msg), 0) < 0) {
        return 1;
    } else {
        return 0;
    }
}
