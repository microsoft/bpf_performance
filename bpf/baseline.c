// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"

// Simplest possible program, just pass everything, used to measure the
// overhead of the BPF infrastructure.

SEC("sockops/baseline") int baseline(void* ctx) { return 0; }
