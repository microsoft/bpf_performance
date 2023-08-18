// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

// Test to measure the overhead of a tail call.

SEC("xdp/tail_callee0") int test_bpf_tail_callee0(struct xdp_md* ctx);

SEC("xdp/tail_callee1") int test_bpf_tail_callee1(struct xdp_md* ctx);

SEC("xdp/tail_callee2") int test_bpf_tail_callee2(struct xdp_md* ctx);

SEC("xdp/tail_callee3") int test_bpf_tail_callee3(struct xdp_md* ctx);

SEC("xdp/tail_callee4") int test_bpf_tail_callee4(struct xdp_md* ctx);

// Prog array map with 5 entry
struct
{
    __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
    __uint(max_entries, 5);
    __uint(key_size, sizeof(__u32));
    __array(values, int(struct xdp_md*));
} prog_array SEC(".maps") = {
    .values =
        {
            [0] = (void*)&test_bpf_tail_callee0,
            [1] = (void*)&test_bpf_tail_callee1,
            [2] = (void*)&test_bpf_tail_callee2,
            [3] = (void*)&test_bpf_tail_callee3,
            [4] = (void*)&test_bpf_tail_callee4,
        },
};

SEC("xdp/tail_callee0") int test_bpf_tail_callee0(struct xdp_md* ctx)
{
    bpf_tail_call(ctx, &prog_array, 1);
    return -1;
}

SEC("xdp/tail_callee1") int test_bpf_tail_callee1(struct xdp_md* ctx)
{
    bpf_tail_call(ctx, &prog_array, 2);
    return -1;
}

SEC("xdp/tail_callee2") int test_bpf_tail_callee2(struct xdp_md* ctx)
{
    bpf_tail_call(ctx, &prog_array, 3);
    return -1;
}

SEC("xdp/tail_callee3") int test_bpf_tail_callee3(struct xdp_md* ctx)
{
    bpf_tail_call(ctx, &prog_array, 4);
    return -1;
}

SEC("xdp/tail_callee4") int test_bpf_tail_callee4(struct xdp_md* ctx) { return 0; }

SEC("xdp/tail_call") int test_bpf_tail(struct xdp_md* ctx)
{
    bpf_tail_call(ctx, &prog_array, 0);
    return -1;
}
