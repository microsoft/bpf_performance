# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

set(CMAKE_CXX_STANDARD 20)

Include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        0.8.0 # or a later release
)

FetchContent_MakeAvailable(yaml-cpp)

include(CheckSymbolExists)

check_symbol_exists(bpf_prog_load "bpf/bpf.h" HAS_BPF_PROG_LOAD)

if(NOT HAS_BPF_PROG_LOAD)
  add_compile_definitions(USE_DEPRECATED_LOAD_PROGRAM)
endif()

add_executable(
  bpf_performance_runner
  runner.cc
)

target_include_directories(bpf_performance_runner PRIVATE ${EBPF_INC_PATH})
target_link_directories(bpf_performance_runner PRIVATE ${EBPF_LIB_PATH})
target_link_libraries(bpf_performance_runner PRIVATE ${EBPF_LIB} "yaml-cpp")
