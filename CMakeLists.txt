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

add_subdirectory(runner)
add_subdirectory(bpf)
