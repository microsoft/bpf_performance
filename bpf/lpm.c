// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "bpf.h"
#include "lpm.h"

#if !defined(MAX_ENTRIES)
#define MAX_ENTRIES 1024
#endif

// Address is stored in network byte order
typedef struct _ipv4_route
{
    unsigned int prefix_length;
    unsigned int address;
} ipv4_route;

struct
{
    __uint(type, BPF_MAP_TYPE_LPM_TRIE);
    __uint(max_entries, MAX_ENTRIES);
    __type(key, ipv4_route);
    __type(value, int);
    __uint(map_flags, BPF_F_NO_PREALLOC);
} lpm_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, MAX_ENTRIES);
    __type(key, int);
    __type(value, ipv4_route);
} lpm_routes_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, int);
    __type(value, int);
} lpm_map_init SEC(".maps");

// Generate and store a collection of random routes with prefix lengths
// that are distributed according to the distribution at https://bgp.potaroo.net/as2.0/bgp-active.html.

SEC("sockops/prepare") int prepare(void* ctx)
{
    unsigned int zero = 0;
    unsigned int* value = bpf_map_lookup_elem(&lpm_map_init, &zero);
    unsigned int index = 0;
    unsigned int address = 0;
    unsigned int prefix_length = 0;

    ipv4_route new_route = {0, 0};
    if (!value || *value >= MAX_ENTRIES) {
        return 0;
    }

    index = *value;

    new_route.prefix_length = select_prefix_length(index, MAX_ENTRIES);
    new_route.address = bpf_get_prandom_u32() & prefix_length_to_network_mask(new_route.prefix_length);

    new_route.address = bpf_htonl(new_route.address);

    if (bpf_map_update_elem(&lpm_map, &new_route, &index, BPF_ANY) < 0) {
        bpf_printk("Failed to insert into lpm_map %x:%x\n", bpf_ntohl(new_route.address), new_route.prefix_length);
        return 1;
    }

    if (bpf_map_update_elem(&lpm_routes_map, &index, &new_route, BPF_ANY) < 0) {
        bpf_printk(
            "Failed to insert into lpm_routes_map  %x:%x\n", bpf_ntohl(new_route.address), new_route.prefix_length);
        return 1;
    }

    *value += 1;
    return 0;
}

SEC("sockops/read") int read(void* ctx)
{
    unsigned int key = bpf_get_prandom_u32() % MAX_ENTRIES;

    ipv4_route* test_route = bpf_map_lookup_elem(&lpm_routes_map, &key);
    ipv4_route test_address = {32, 0};

    if (!test_route) {
        bpf_printk("Failed to lookup route %d\n", key);
        return 1;
    }

    test_address.address = prefix_length_to_host_mask(test_route->prefix_length) & bpf_get_prandom_u32();
    test_address.address |= bpf_ntohl(test_route->address);

    test_address.address = bpf_htonl(test_address.address);

    unsigned int* result = bpf_map_lookup_elem(&lpm_map, &test_address);
    if (!result) {
        bpf_printk(
            "Failed to lookup route in lpm_map %x:%x\n", bpf_ntohl(test_address.address), test_address.prefix_length);
        bpf_printk("Built from route %x:%x\n", bpf_ntohl(test_route->address), test_route->prefix_length);
        return 1;
    }

    unsigned int index = *result;
    ipv4_route* result_route = bpf_map_lookup_elem(&lpm_routes_map, &index);
    if (!result_route) {
        bpf_printk("Failed to lookup route in lpm_routes_map %d\n", index);
        return 1;
    }

    unsigned int test_address_network =
        bpf_ntohl(test_address.address) & prefix_length_to_network_mask(result_route->prefix_length);
    if (test_address_network != bpf_ntohl(result_route->address)) {
        bpf_printk("Failed to match route %x:%x\n", bpf_ntohl(test_address.address), test_address.prefix_length);
        bpf_printk("Built from route %x:%x\n", bpf_ntohl(test_route->address), test_route->prefix_length);
        bpf_printk("Result route %x:%x\n", bpf_ntohl(result_route->address), result_route->prefix_length);
        return 1;
    }

    return 0;
}

SEC("sockops/update") int update(void* ctx)
{
    unsigned int key = bpf_get_prandom_u32() % MAX_ENTRIES;

    ipv4_route* test_route = bpf_map_lookup_elem(&lpm_routes_map, &key);
    ipv4_route route_key = {32, 0};

    if (!test_route) {
        return 1;
    }
    route_key = *test_route;

    (void)bpf_map_update_elem(&lpm_map, &route_key, &key, BPF_ANY);

    return 0;
}

SEC("sockops/replace") int replace(void* ctx)
{
    unsigned int key = bpf_get_prandom_u32() % MAX_ENTRIES;

    ipv4_route* test_route = bpf_map_lookup_elem(&lpm_routes_map, &key);
    ipv4_route route_key = {32, 0};

    if (!test_route) {
        return 1;
    }
    route_key = *test_route;

    (void)bpf_map_delete_elem(&lpm_map, &route_key);
    (void)bpf_map_update_elem(&lpm_map, &route_key, &key, BPF_ANY);

    return 0;
}
