// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

SEC("xdp/baseline") int test_xdp_baseline(void* ctx) { return XDP_PASS; }

// Test cases for bpf_xdp_adjust_head

SEC("xdp/test_bpf_xdp_adjust_head_0") int test_bpf_xdp_adjust_head_0(void* ctx)
{
    if (bpf_xdp_adjust_head(ctx, 0) < 0) {
        return XDP_DROP;
    }
    return XDP_PASS;
}

SEC("xdp/test_bpf_xdp_adjust_head_plus_100") int test_bpf_xdp_adjust_head_plus_100(void* ctx)
{
    if (bpf_xdp_adjust_head(ctx, 100) < 0) {
        return XDP_DROP;
    }

    if (bpf_xdp_adjust_head(ctx, -100) < 0) {
        return XDP_DROP;
    }
    return XDP_PASS;
}

SEC("xdp/test_bpf_xdp_adjust_head_minus_100") int test_bpf_xdp_adjust_head_minus_100(void* ctx)
{
    if (bpf_xdp_adjust_head(ctx, -100) < 0) {
        return XDP_DROP;
    }
    if (bpf_xdp_adjust_head(ctx, 100) < 0) {
        return XDP_DROP;
    }
    return XDP_PASS;
}
