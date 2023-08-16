// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

// Simplest possible program, just pass everything, used to measure the
// overhead of the BPF infrastructure.

__attribute__((section("xdp/baseline"))) int
baseline_program(void* ctx)
{
    return 0;
}
