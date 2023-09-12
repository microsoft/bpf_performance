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

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
