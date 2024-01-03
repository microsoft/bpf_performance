// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: GPL/MIT

#include "bpf.h"

// Get performance numbers for the scaled maximum tail call callee count,
// without the caller.

#define MAX_TAIL_CALL_COUNT 32

// Define a macro that defines a program which tail calls a function for the hook.
#define DEFINE_TAIL_FUNC(x)                              \
    SEC("sockops/" #x)                                   \
    int test_callee##x(void* ctx)                        \
    {                                                    \
        int i = x + 1;                                   \
        bpf_tail_call(ctx, &tail_call_map, i);           \
        bpf_printk("Tail call failed at index %d\n", i); \
                                                         \
        return -1;                                       \
    }

#define DECLARE_TAIL_FUNC(x) int test_callee##x(void* ctx);

DECLARE_TAIL_FUNC(0)
DECLARE_TAIL_FUNC(1)
DECLARE_TAIL_FUNC(2)
DECLARE_TAIL_FUNC(3)
DECLARE_TAIL_FUNC(4)
DECLARE_TAIL_FUNC(5)
DECLARE_TAIL_FUNC(6)
DECLARE_TAIL_FUNC(7)
DECLARE_TAIL_FUNC(8)
DECLARE_TAIL_FUNC(9)
DECLARE_TAIL_FUNC(10)
DECLARE_TAIL_FUNC(11)
DECLARE_TAIL_FUNC(12)
DECLARE_TAIL_FUNC(13)
DECLARE_TAIL_FUNC(14)
DECLARE_TAIL_FUNC(15)
DECLARE_TAIL_FUNC(16)
DECLARE_TAIL_FUNC(17)
DECLARE_TAIL_FUNC(18)
DECLARE_TAIL_FUNC(19)
DECLARE_TAIL_FUNC(20)
DECLARE_TAIL_FUNC(21)
DECLARE_TAIL_FUNC(22)
DECLARE_TAIL_FUNC(23)
DECLARE_TAIL_FUNC(24)
DECLARE_TAIL_FUNC(25)
DECLARE_TAIL_FUNC(26)
DECLARE_TAIL_FUNC(27)
DECLARE_TAIL_FUNC(28)
DECLARE_TAIL_FUNC(29)
DECLARE_TAIL_FUNC(30)
DECLARE_TAIL_FUNC(31)
DECLARE_TAIL_FUNC(32)
DECLARE_TAIL_FUNC(33)
DECLARE_TAIL_FUNC(34)

// Number of tail calls is 35, without the caller.
struct
{
    __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
    __uint(key_size, sizeof(__u32));
    __uint(max_entries, MAX_TAIL_CALL_COUNT + 3);
    __array(values, int(void* ctx));
} tail_call_map SEC(".maps") = {
    .values = {
        test_callee0,  test_callee1,  test_callee2,  test_callee3,  test_callee4,  test_callee5,  test_callee6,
        test_callee7,  test_callee8,  test_callee9,  test_callee10, test_callee11, test_callee12, test_callee13,
        test_callee14, test_callee15, test_callee16, test_callee17, test_callee18, test_callee19, test_callee20,
        test_callee21, test_callee22, test_callee23, test_callee24, test_callee25, test_callee26, test_callee27,
        test_callee28, test_callee29, test_callee30, test_callee31, test_callee32, test_callee33, test_callee34,
    }};

DEFINE_TAIL_FUNC(0)
DEFINE_TAIL_FUNC(1)
DEFINE_TAIL_FUNC(2)
DEFINE_TAIL_FUNC(3)
DEFINE_TAIL_FUNC(4)
DEFINE_TAIL_FUNC(5)
DEFINE_TAIL_FUNC(6)
DEFINE_TAIL_FUNC(7)
DEFINE_TAIL_FUNC(8)
DEFINE_TAIL_FUNC(9)
DEFINE_TAIL_FUNC(10)
DEFINE_TAIL_FUNC(11)
DEFINE_TAIL_FUNC(12)
DEFINE_TAIL_FUNC(13)
DEFINE_TAIL_FUNC(14)
DEFINE_TAIL_FUNC(15)
DEFINE_TAIL_FUNC(16)
DEFINE_TAIL_FUNC(17)
DEFINE_TAIL_FUNC(18)
DEFINE_TAIL_FUNC(19)
DEFINE_TAIL_FUNC(20)
DEFINE_TAIL_FUNC(21)
DEFINE_TAIL_FUNC(22)
DEFINE_TAIL_FUNC(23)
DEFINE_TAIL_FUNC(24)
DEFINE_TAIL_FUNC(25)
DEFINE_TAIL_FUNC(26)
DEFINE_TAIL_FUNC(27)
DEFINE_TAIL_FUNC(28)
DEFINE_TAIL_FUNC(29)
DEFINE_TAIL_FUNC(30)
DEFINE_TAIL_FUNC(31)
DEFINE_TAIL_FUNC(32)
DEFINE_TAIL_FUNC(33)

SEC("sockops/first")
int
test_caller(void* ctx)
{
    bpf_tail_call(ctx, &tail_call_map, 0);
    bpf_printk("Failed Tail call index %d\n", 0);
    return -1;
}

SEC("sockops/34")
int
test_callee34(void* ctx)
{
    // This function is the last tail call function for the xdp hook.
    // This function returns 0 to allow the xdp request to proceed.
    return 0;
}

char _license[] SEC("license") = "GPL/MIT";
