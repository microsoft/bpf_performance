// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

// Prefix Length Distributions from https://bgp.potaroo.net/as2.0/bgp-active.html
// Cumulative count of prefix lengths, where each entry represents the sum of all entries before it.
#define SCALED_CUMULATIVE_COUNT_0(MAX_ENTRIES) ((0ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_1(MAX_ENTRIES) ((0ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_2(MAX_ENTRIES) ((0ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_3(MAX_ENTRIES) ((0ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_4(MAX_ENTRIES) ((0ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_5(MAX_ENTRIES) ((0ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_6(MAX_ENTRIES) ((0ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_7(MAX_ENTRIES) ((16ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_8(MAX_ENTRIES) ((29ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_9(MAX_ENTRIES) ((70ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_10(MAX_ENTRIES) ((172ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_11(MAX_ENTRIES) ((478ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_12(MAX_ENTRIES) ((1074ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_13(MAX_ENTRIES) ((2289ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_14(MAX_ENTRIES) ((4379ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_15(MAX_ENTRIES) ((18026ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_16(MAX_ENTRIES) ((26417ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_17(MAX_ENTRIES) ((40633ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_18(MAX_ENTRIES) ((66374ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_19(MAX_ENTRIES) ((110039ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_20(MAX_ENTRIES) ((163137ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_21(MAX_ENTRIES) ((272418ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_22(MAX_ENTRIES) ((370199ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_23(MAX_ENTRIES) ((894075ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_24(MAX_ENTRIES) ((895534ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_25(MAX_ENTRIES) ((895534ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_26(MAX_ENTRIES) ((895534ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_27(MAX_ENTRIES) ((895535ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_28(MAX_ENTRIES) ((895535ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_29(MAX_ENTRIES) ((895536ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_30(MAX_ENTRIES) ((895536ull * MAX_ENTRIES) / 895537ull)
#define SCALED_CUMULATIVE_COUNT_31(MAX_ENTRIES) ((895537ull * MAX_ENTRIES) / 895537ull)

// Given a index within range [0, scale), return the prefix length that corresponds to that index.
// Pref
static inline unsigned int
select_prefix_length(unsigned long index, unsigned long scale)
{
    if (SCALED_CUMULATIVE_COUNT_0(scale) > index) {
        return 0;
    }
    if (SCALED_CUMULATIVE_COUNT_1(scale) > index) {
        return 1;
    }
    if (SCALED_CUMULATIVE_COUNT_2(scale) > index) {
        return 2;
    }
    if (SCALED_CUMULATIVE_COUNT_3(scale) > index) {
        return 3;
    }
    if (SCALED_CUMULATIVE_COUNT_4(scale) > index) {
        return 4;
    }
    if (SCALED_CUMULATIVE_COUNT_5(scale) > index) {
        return 5;
    }
    if (SCALED_CUMULATIVE_COUNT_6(scale) > index) {
        return 6;
    }
    if (SCALED_CUMULATIVE_COUNT_7(scale) > index) {
        return 7;
    }
    if (SCALED_CUMULATIVE_COUNT_8(scale) > index) {
        return 8;
    }
    if (SCALED_CUMULATIVE_COUNT_9(scale) > index) {
        return 9;
    }
    if (SCALED_CUMULATIVE_COUNT_10(scale) > index) {
        return 10;
    }
    if (SCALED_CUMULATIVE_COUNT_11(scale) > index) {
        return 11;
    }
    if (SCALED_CUMULATIVE_COUNT_12(scale) > index) {
        return 12;
    }
    if (SCALED_CUMULATIVE_COUNT_13(scale) > index) {
        return 13;
    }
    if (SCALED_CUMULATIVE_COUNT_14(scale) > index) {
        return 14;
    }
    if (SCALED_CUMULATIVE_COUNT_15(scale) > index) {
        return 15;
    }
    if (SCALED_CUMULATIVE_COUNT_16(scale) > index) {
        return 16;
    }
    if (SCALED_CUMULATIVE_COUNT_17(scale) > index) {
        return 17;
    }
    if (SCALED_CUMULATIVE_COUNT_18(scale) > index) {
        return 18;
    }
    if (SCALED_CUMULATIVE_COUNT_19(scale) > index) {
        return 19;
    }
    if (SCALED_CUMULATIVE_COUNT_20(scale) > index) {
        return 20;
    }
    if (SCALED_CUMULATIVE_COUNT_21(scale) > index) {
        return 21;
    }
    if (SCALED_CUMULATIVE_COUNT_22(scale) > index) {
        return 22;
    }
    if (SCALED_CUMULATIVE_COUNT_23(scale) > index) {
        return 23;
    }
    if (SCALED_CUMULATIVE_COUNT_24(scale) > index) {
        return 24;
    }
    if (SCALED_CUMULATIVE_COUNT_25(scale) > index) {
        return 25;
    }
    if (SCALED_CUMULATIVE_COUNT_26(scale) > index) {
        return 26;
    }
    if (SCALED_CUMULATIVE_COUNT_27(scale) > index) {
        return 27;
    }
    if (SCALED_CUMULATIVE_COUNT_28(scale) > index) {
        return 28;
    }
    if (SCALED_CUMULATIVE_COUNT_29(scale) > index) {
        return 29;
    }
    if (SCALED_CUMULATIVE_COUNT_30(scale) > index) {
        return 30;
    }
    if (SCALED_CUMULATIVE_COUNT_31(scale) > index) {
        return 31;
    }
    return 32;
}

static inline unsigned int
prefix_length_to_network_mask(unsigned int prefix_length)
{
    return (0xFFFFFFFF << (32 - prefix_length));
}

static inline unsigned int
prefix_length_to_host_mask(unsigned int prefix_length)
{
    return (0xFFFFFFFF >> prefix_length);
}
