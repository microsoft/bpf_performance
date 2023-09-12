# bpf_performance

[![CI/CD](https://github.com/microsoft/bpf_performance/actions/workflows/CICD.yml/badge.svg)](https://github.com/microsoft/bpf_performance/actions/workflows/CICD.yml)
[![Coverage Status](https://coveralls.io/repos/github/microsoft/bpf_performance/badge.svg?branch=main)](https://coveralls.io/github/microsoft/bpf_performance?branch=main)

A set of platform-agnostic tools to measure the performance of various BPF helper functions.

## Overview

Tests are defined as BPF programs, each consisting of a `map_state_preparation` phase and a `test` phase. These tests are described in a YAML file.

As an example:

```yaml
tests:
  - name: Baseline
    description: The Baseline test with an empty eBPF program.
    elf_file: baseline.o
    iteration_count: 10000000
    program_cpu_assignment:
      baseline_program: all

  - name: Hash-table Map Read
    description: Tests reading from a BPF_MAP_TYPE_HASH map.
    elf_file: generic_map.o
    map_state_preparation:
      program: prepare_HASH_state_generic_read
      iteration_count: 1024
    iteration_count: 10000000
    program_cpu_assignment:
      read_HASH: all

  - name: Hash-table Map Update / Read
    description: Tests reading and writing to a BPF_MAP_TYPE_HASH map.
    elf_file: generic_map.o
    map_state_preparation:
      program: prepare_HASH_state_generic_read
      iteration_count: 1024
    iteration_count: 10000000
    program_cpu_assignment:
      update_HASH: [0]
      read_HASH: remaining
```

Test programs can be pinned to specific CPUs to permit mixed behavior tests, such as concurrent reads and updates to a map.

## Building

To build the project:

```shell
cmake -B build -S .
cmake --build build
```

This build process works on both Windows (using eBPF for Windows) and Linux.

For Linux, you need the following packages:
1. gcc-multilib
2. libbpf-dev
3. Clang / llvm

For Windows, you need the following tools:
1. Nuget
2. Clang / LLVM

## Running the tests

To run the tests on Linux:

```shell
cd build/bin
sudo ./bpf_performance_runner tests.yml
```

On Windows (after installing eBPF for Windows MSI):

```cmd
cd build\bin\Debug
.\bpf_performance_runner tests.yml
```
