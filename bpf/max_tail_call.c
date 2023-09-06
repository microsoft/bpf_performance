// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: GPL/MIT

#include "bpf.h"

// Get performance numbers for the scaled maximum tail call callee count,
// without the caller.

#define MAX_TAIL_CALL_COUNT 32

// Define a macro that defines a program which tail calls a function for the xdp hook.
#define DEFINE_XDP_TAIL_FUNC(x)                          \
    SEC("xdp/" #x)                                       \
    int xdp_test_callee##x(struct xdp_md* ctx)           \
    {                                                    \
        int i = x + 1;                                   \
        bpf_tail_call(ctx, &xdp_tail_call_map, i);       \
        bpf_printk("Tail call failed at index %d\n", i); \
                                                         \
        return -1;                                       \
    }

#define DECLARE_XDP_TAIL_FUNC(x) int xdp_test_callee##x(struct xdp_md* ctx);

DECLARE_XDP_TAIL_FUNC(0)
DECLARE_XDP_TAIL_FUNC(1)
DECLARE_XDP_TAIL_FUNC(2)
DECLARE_XDP_TAIL_FUNC(3)
DECLARE_XDP_TAIL_FUNC(4)
DECLARE_XDP_TAIL_FUNC(5)
DECLARE_XDP_TAIL_FUNC(6)
DECLARE_XDP_TAIL_FUNC(7)
DECLARE_XDP_TAIL_FUNC(8)
DECLARE_XDP_TAIL_FUNC(9)
DECLARE_XDP_TAIL_FUNC(10)
DECLARE_XDP_TAIL_FUNC(11)
DECLARE_XDP_TAIL_FUNC(12)
DECLARE_XDP_TAIL_FUNC(13)
DECLARE_XDP_TAIL_FUNC(14)
DECLARE_XDP_TAIL_FUNC(15)
DECLARE_XDP_TAIL_FUNC(16)
DECLARE_XDP_TAIL_FUNC(17)
DECLARE_XDP_TAIL_FUNC(18)
DECLARE_XDP_TAIL_FUNC(19)
DECLARE_XDP_TAIL_FUNC(20)
DECLARE_XDP_TAIL_FUNC(21)
DECLARE_XDP_TAIL_FUNC(22)
DECLARE_XDP_TAIL_FUNC(23)
DECLARE_XDP_TAIL_FUNC(24)
DECLARE_XDP_TAIL_FUNC(25)
DECLARE_XDP_TAIL_FUNC(26)
DECLARE_XDP_TAIL_FUNC(27)
DECLARE_XDP_TAIL_FUNC(28)
DECLARE_XDP_TAIL_FUNC(29)
DECLARE_XDP_TAIL_FUNC(30)
DECLARE_XDP_TAIL_FUNC(31)
DECLARE_XDP_TAIL_FUNC(32)
DECLARE_XDP_TAIL_FUNC(33)
DECLARE_XDP_TAIL_FUNC(34)

// Number of tail calls is 35, without the caller.
struct
{
    __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
    __uint(key_size, sizeof(__u32));
    __uint(max_entries, MAX_TAIL_CALL_COUNT + 3);
    __array(values, int(struct xdp_md* ctx));
} xdp_tail_call_map SEC(".maps") = {
    .values = {
        xdp_test_callee0,  xdp_test_callee1,  xdp_test_callee2,  xdp_test_callee3,
        xdp_test_callee4,  xdp_test_callee5,  xdp_test_callee6,  xdp_test_callee7,
        xdp_test_callee8,  xdp_test_callee9,  xdp_test_callee10, xdp_test_callee11,
        xdp_test_callee12, xdp_test_callee13, xdp_test_callee14, xdp_test_callee15,
        xdp_test_callee16, xdp_test_callee17, xdp_test_callee18, xdp_test_callee19,
        xdp_test_callee20, xdp_test_callee21, xdp_test_callee22, xdp_test_callee23,
        xdp_test_callee24, xdp_test_callee25, xdp_test_callee26, xdp_test_callee27,
        xdp_test_callee28, xdp_test_callee29, xdp_test_callee30, xdp_test_callee31,
        xdp_test_callee32, xdp_test_callee33, xdp_test_callee34,
    }};

DEFINE_XDP_TAIL_FUNC(0)
DEFINE_XDP_TAIL_FUNC(1)
DEFINE_XDP_TAIL_FUNC(2)
DEFINE_XDP_TAIL_FUNC(3)
DEFINE_XDP_TAIL_FUNC(4)
DEFINE_XDP_TAIL_FUNC(5)
DEFINE_XDP_TAIL_FUNC(6)
DEFINE_XDP_TAIL_FUNC(7)
DEFINE_XDP_TAIL_FUNC(8)
DEFINE_XDP_TAIL_FUNC(9)
DEFINE_XDP_TAIL_FUNC(10)
DEFINE_XDP_TAIL_FUNC(11)
DEFINE_XDP_TAIL_FUNC(12)
DEFINE_XDP_TAIL_FUNC(13)
DEFINE_XDP_TAIL_FUNC(14)
DEFINE_XDP_TAIL_FUNC(15)
DEFINE_XDP_TAIL_FUNC(16)
DEFINE_XDP_TAIL_FUNC(17)
DEFINE_XDP_TAIL_FUNC(18)
DEFINE_XDP_TAIL_FUNC(19)
DEFINE_XDP_TAIL_FUNC(20)
DEFINE_XDP_TAIL_FUNC(21)
DEFINE_XDP_TAIL_FUNC(22)
DEFINE_XDP_TAIL_FUNC(23)
DEFINE_XDP_TAIL_FUNC(24)
DEFINE_XDP_TAIL_FUNC(25)
DEFINE_XDP_TAIL_FUNC(26)
DEFINE_XDP_TAIL_FUNC(27)
DEFINE_XDP_TAIL_FUNC(28)
DEFINE_XDP_TAIL_FUNC(29)
DEFINE_XDP_TAIL_FUNC(30)
DEFINE_XDP_TAIL_FUNC(31)
DEFINE_XDP_TAIL_FUNC(32)
DEFINE_XDP_TAIL_FUNC(33)

SEC("xdp")
int
xdp_test_caller(struct xdp_md* ctx)
{
    bpf_tail_call(ctx, &xdp_tail_call_map, 0);
    bpf_printk("Failed Tail call index %d\n", 0);
    return -1;
}

SEC("xdp/34")
int
xdp_test_callee34(struct xdp_md* ctx)
{
    // This function is the last tail call function for the xdp hook.
    // This function returns 0 to allow the xdp request to proceed.
    return 0;
}

char _license[] SEC("license") = "GPL/MIT";
