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
    # if BPF_PERF_LOCAL_NUGET_PATH is set, use that as the source for the eBPF-for-Windows package
    if(BPF_PERF_LOCAL_NUGET_PATH)
      exec_program(${NUGET} ARGS install "eBPF-for-Windows" -Version 0.10.0 -ExcludeVersion -OutputDirectory ${PROJECT_BINARY_DIR}/packages -Source ${BPF_PERF_LOCAL_NUGET_PATH} -NoCache)
    else()
      exec_program(${NUGET} ARGS install "eBPF-for-Windows" -Version 0.10.0 -ExcludeVersion -OutputDirectory ${PROJECT_BINARY_DIR}/packages)
    endif()
  endif()
  set(EBPF_LIB "ebpfapi")
  set(EBPF_INC_PATH "${PROJECT_BINARY_DIR}/packages/eBPF-for-Windows/build/native/include")
  set(EBPF_LIB_PATH "${PROJECT_BINARY_DIR}/packages/eBPF-for-Windows/build/native/lib")
  set(EBPF_BIN_PATH "${PROJECT_BINARY_DIR}/packages/eBPF-for-Windows/build/native/bin")
elseif(PLATFORM_LINUX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBBPF REQUIRED libbpf)
  set(EBPF_LIB ${LIBBPF_LIBRARIES})
  set(EBPF_INC_PATH ${LIBBPF_INCLUDEDIR})
  set(EBPF_LIB_PATH ${LIBBPF_LIBDIR})
  # Print out the libbpf version
  message(STATUS "libbpf version: ${LIBBPF_VERSION}")
  message(STATUS "libbpf include dirs: ${LIBBPF_INCLUDEDIR}")
  message(STATUS "libbpf library dirs: ${LIBBPF_LIBDIR}")
  message(STATUS "libbpf libraries: ${LIBBPF_LIBRARIES}")

endif()

add_subdirectory(runner)
add_subdirectory(bpf)

enable_testing()

file(COPY ${CMAKE_SOURCE_DIR}/runner/tests DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Add test for usage message
add_test(
  NAME usage
  COMMAND sudo bin/bpf_performance_runner
)

# Mark test as expected to fail with "Usage: bpf_performance_runner <config_file>"
set_tests_properties(
  usage PROPERTIES
  PASS_REGULAR_EXPRESSION "Usage: .*"
)

# Test for empty yaml file
add_test(
  NAME empty_yaml
  COMMAND sudo bin/bpf_performance_runner bin/tests/empty.yaml
)

# Mark test as expected to fail with "Error: bad file: tests/empty.yaml"
set_tests_properties(
  empty_yaml PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Invalid config file - tests must be a sequence"
)

# Test for missing yaml file
add_test(
  NAME missing_yaml
  COMMAND sudo bin/bpf_performance_runner bin/tests/missing.yaml
)

# Mark test as expected to fail with "Error: bad file: tests/missing.yaml"
set_tests_properties(
  missing_yaml PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: bad file: bin/tests/missing.yaml"
)

# Test for missing "name" field
add_test(
  NAME missing_name
  COMMAND sudo bin/bpf_performance_runner bin/tests/missing_name.yaml
)

# Mark test as expected to fail with "Error: Field name is required"
set_tests_properties(
  missing_name PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field name is required"
)

# Test for missing "elf_file" field
add_test(
  NAME missing_elf_file
  COMMAND sudo bin/bpf_performance_runner bin/tests/missing_elf_file.yaml
)

# Mark test as expected to fail with "Error: Field elf_file is required"
set_tests_properties(
  missing_elf_file PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field elf_file is required"
)

# Test for missing "iteration_count" field
add_test(
  NAME missing_iteration_count
  COMMAND sudo bin/bpf_performance_runner bin/tests/missing_iteration_count.yaml
)

# Mark test as expected to fail with "Error: Field iteration_count is required"
set_tests_properties(
  missing_iteration_count PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field iteration_count is required"
)

# Test for missing "program_cpu_assignment" field
add_test(
  NAME missing_program_cpu_assignment
  COMMAND sudo bin/bpf_performance_runner bin/tests/missing_program_cpu_assignment.yaml
)

# Mark test as expected to fail with "Error: Field program_cpu_assignment is required"
set_tests_properties(
  missing_program_cpu_assignment PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field program_cpu_assignment is required"
)

# Test for empty "program_cpu_assignment" field
add_test(
  NAME empty_program_cpu_assignment
  COMMAND sudo bin/bpf_performance_runner bin/tests/empty_program_cpu_assignment.yaml
)

# Mark test as expected to fail with "Error: Field program_cpu_assignment must be a map"
set_tests_properties(
  empty_program_cpu_assignment PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field program_cpu_assignment must be a map"
)

# Disable this test until https://github.com/Alan-Jowett/bpf_performance/issues/8 is fixed.
# # Test for elf file not found
# add_test(
#   NAME elf_file_not_found
#   COMMAND sudo bin/bpf_performance_runner bin/tests/elf_file_not_found.yaml
# )

# # Mark test as expected to fail with "libbpf: elf: failed to open not_a_file.o: No such file or directory"
# set_tests_properties(
#   elf_file_not_found PROPERTIES
#   PASS_REGULAR_EXPRESSION "libbpf: elf: failed to open not_a_file.o: No such file or directory"
# )

# Test for function map_state_preparation not found
add_test(
  NAME function_map_state_preparation_not_found
  COMMAND sudo bin/bpf_performance_runner bin/tests/function_map_state_preparation_not_found.yaml
)

# Mark test as expected to fail with "Error: Failed to find map_state_preparation program not_a_real_function"
set_tests_properties(
  function_map_state_preparation_not_found PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Failed to find map_state_preparation program not_a_real_function"
)

# Test for map_state_preparation.program not found
add_test(
  NAME map_state_preparation_program_not_found
  COMMAND sudo bin/bpf_performance_runner bin/tests/map_state_preparation_program_not_found.yaml
)

# Mark test as expected to fail with "Error: Field map_state_preparation.program is required"
set_tests_properties(
  map_state_preparation_program_not_found PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field map_state_preparation.program is required"
)

# Test for map_state_preparation.iteration_count not found
add_test(
  NAME map_state_preparation_iteration_count_not_found
  COMMAND sudo bin/bpf_performance_runner bin/tests/map_state_preparation_iteration_count_not_found.yaml
)

# Mark test as expected to fail with "Error: Field map_state_preparation.iteration_count is required"
set_tests_properties(
  map_state_preparation_iteration_count_not_found PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field map_state_preparation.iteration_count is required"
)

# Test for program_cpu_assignment.program not found
add_test(
  NAME function_program_cpu_assignment_not_found
  COMMAND sudo bin/bpf_performance_runner bin/tests/function_program_cpu_assignment_not_found.yaml
)

# Mark test as expected to fail with "Error: Failed to find program not_a_function"
set_tests_properties(
  function_program_cpu_assignment_not_found PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Failed to find program not_a_function"
)

# Test for invalid program_cpu_assignment value
add_test(
  NAME invalid_program_cpu_assignment_value
  COMMAND sudo bin/bpf_performance_runner bin/tests/invalid_program_cpu_assignment_value.yaml
)

# Mark test as expected to fail with "Error: Invalid program_cpu_assignment - must be string or sequence"
set_tests_properties(
  invalid_program_cpu_assignment_value PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Invalid program_cpu_assignment - must be string or sequence"
)