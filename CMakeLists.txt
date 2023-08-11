# Copyright (c) 2023 Microsoft Corporation.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)
project("bpf_performance")

if (BPF_PERF_INSTALL_GIT_HOOKS AND EXISTS "${PROJECT_SOURCE_DIR}/.git/hooks")
    # Install Git pre-commit hook
    file(COPY scripts/pre-commit scripts/commit-msg
        DESTINATION "${PROJECT_SOURCE_DIR}/.git/hooks")
endif()

include("cmake/platform.cmake")
include("cmake/options.cmake")
include("cmake/version.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

if (PLATFORM_WINDOWS)
  find_program(NUGET nuget)
  if(NOT NUGET)
    message("ERROR: You must first install nuget.exe from https://www.nuget.org/downloads")
  else()
    exec_program(${NUGET} ARGS install "eBPF-for-Windows" -Version 0.10.0 -ExcludeVersion -OutputDirectory ${PROJECT_BINARY_DIR}/packages)
  endif()
  set(EBPF_LIB "ebpfapi")
  set(EBPF_INC_PATH "${PROJECT_BINARY_DIR}/packages/eBPF-for-Windows/build/native/include")
  set(EBPF_LIB_PATH "${PROJECT_BINARY_DIR}/packages/eBPF-for-Windows/build/native/lib")
elseif(PLATFORM_LINUX)
  find_library(LIBBPF libbpf)
  set(EBPF_LIB "bpf")
  set(EBPF_INC_PATH "/usr/include")
endif()

add_subdirectory(runner)
add_subdirectory(bpf)
