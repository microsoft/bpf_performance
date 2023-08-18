// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

// Test cases for bpf_xdp_adjust_head

__attribute__((section("xdp/test_bpf_xdp_adjust_head_0"))) int
test_bpf_xdp_adjust_head_0(struct xdp_md* ctx)
{
    if (bpf_xdp_adjust_head(ctx, 0) < 0) {
        return -1;
    }
    return 0;
}

__attribute__((section("xdp/test_bpf_xdp_adjust_head_plus_100"))) int
test_bpf_xdp_adjust_head_plus_100(struct xdp_md* ctx)
{
    if (bpf_xdp_adjust_head(ctx, 100) < 0) {
        return -1;
    }

    if (bpf_xdp_adjust_head(ctx, -100) < 0) {
        return -1;
    }
    return 0;
}

__attribute__((section("xdp/test_bpf_xdp_adjust_head_minus_100"))) int
test_bpf_xdp_adjust_head_minus_100(struct xdp_md* ctx)
{
    if (bpf_xdp_adjust_head(ctx, -100) < 0) {
        return -1;
    }
    if (bpf_xdp_adjust_head(ctx, 100) < 0) {
        return -1;
    }
    return 0;
}
