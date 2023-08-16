// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

// Simplest possible program, just pass everything, used to measure the
// overhead of the BPF infrastructure.

__attribute__((section("xdp/test_bpf_get_prandom_u32"))) int
test_bpf_get_prandom_u32(void* ctx)
{
    int i = bpf_get_prandom_u32();
    return 0;
}
