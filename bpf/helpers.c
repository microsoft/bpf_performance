// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

// Test to measure the overhead of bpf_get_prandom_u32
SEC("xdp/test_bpf_get_prandom_u32") int test_bpf_get_prandom_u32(void* ctx)
{
    int i = bpf_get_prandom_u32();
    return 0;
}
