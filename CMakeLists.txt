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


if (PLATFORM_WINDOWS)
  find_program(NUGET nuget)
  if(NOT NUGET)
    message("ERROR: You must first install nuget.exe from https://www.nuget.org/downloads")
  else()
    # if BPF_PERF_LOCAL_NUGET_PATH is set, use that as the source for the eBPF-for-Windows package
    if(BPF_PERF_LOCAL_NUGET_PATH)
      set(EBPF_PACKAGE_NAME "eBPF-for-Windows.x64")
      exec_program(${NUGET} ARGS install "eBPF-for-Windows.x64" -ExcludeVersion -OutputDirectory ${PROJECT_BINARY_DIR}/packages -Source ${BPF_PERF_LOCAL_NUGET_PATH} -NoCache)
    else()
      # Update this once the new version of eBPF-for-Windows is released.
      set(EBPF_PACKAGE_NAME "eBPF-for-Windows.x64")
      exec_program(${NUGET} ARGS install "eBPF-for-Windows.x64" -Version 0.21.1 -ExcludeVersion -OutputDirectory ${PROJECT_BINARY_DIR}/packages)
    endif()
  endif()
  set(EBPF_LIB "ebpfapi")
  set(EBPF_INC_PATH "${PROJECT_BINARY_DIR}/packages/${EBPF_PACKAGE_NAME}/build/native/include")
  set(EBPF_LIB_PATH "${PROJECT_BINARY_DIR}/packages/${EBPF_PACKAGE_NAME}/build/native/lib")
  set(EBPF_BIN_PATH "${PROJECT_BINARY_DIR}/packages/${EBPF_PACKAGE_NAME}/build/native/bin")

  # Run  export_program_info.exe from the eBPF-for-Windows package
  execute_process(COMMAND "${EBPF_BIN_PATH}/export_program_info.exe" WORKING_DIRECTORY "${EBPF_BIN_PATH}" COMMAND_ERROR_IS_FATAL ANY)

  if (BPF_XDP_TEST_ENABLED)
    # Download the XDP-dev-kit from GitHub
    file(READ "${CMAKE_SOURCE_DIR}/scripts/xdp_version.txt" XDP_VERSION)
    file(DOWNLOAD "${XDP_VERSION}/xdp-devkit-x64-1.1.0.zip" "${PROJECT_BINARY_DIR}/packages/xdp-devkit.zip")
    # Unzip the XDP-dev-kit
    # Create folder xdp-devkit
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/packages/xdp-devkit")
    # Unzip the xdp-devkit.zip file into the xdp-devkit folder
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${PROJECT_BINARY_DIR}/packages/xdp-devkit.zip" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/packages/xdp-devkit")

    SET(XDP_INC_PATH "${PROJECT_BINARY_DIR}/packages/xdp-devkit/include")
    SET(XDP_LIB_PATH "${PROJECT_BINARY_DIR}/packages/xdp-devkit/lib")
    SET(XDP_BIN_PATH "${PROJECT_BINARY_DIR}/packages/xdp-devkit/bin")
    # Run xdp_bpfexport.exe from the xdp-dev-kit
    execute_process(COMMAND "${PROJECT_BINARY_DIR}/packages/xdp-devkit/bin/xdp_bpfexport.exe" WORKING_DIRECTORY "${EBPF_BIN_PATH}" COMMAND_ERROR_IS_FATAL ANY)
  endif()
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

if(PLATFORM_WINDOWS)
message("Setting CONTROL FLOW GUARD")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")
SET(CMAKE_EXE_LINKER_FLAGS  "/guard:cf /DYNAMICBASE")

message("Setting QSPECTRE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Qspectre")
endif()

add_subdirectory(runner)
add_subdirectory(bpf)

enable_testing()

# Specify the directory you want to create
set(tests_directory "${PROJECT_BINARY_DIR}/tests")

# Check if the directory exists
if(NOT EXISTS ${tests_directory})
    # Create the directory if it doesn't exist
    file(MAKE_DIRECTORY ${tests_directory})
endif()

# Add test for usage message
add_test(
  NAME usage
  COMMAND sudo bin/bpf_performance_runner -?
)

# Mark test as expected to fail with "Usage: bpf_performance_runner <config_file>"
set_tests_properties(
  usage PROPERTIES
  PASS_REGULAR_EXPRESSION "Usage: .*"
)

set(TEST_FILE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runner/tests")

# Test for empty yaml file
add_test(
  NAME empty_yaml
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/empty.yaml
)

# Mark test as expected to fail with "Error: bad file: tests/empty.yaml"
set_tests_properties(
  empty_yaml PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Invalid config file - tests must be a sequence"
)

# Test for missing yaml file
add_test(
  NAME missing_yaml
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/missing.yaml
)

# Mark test as expected to fail with "Error: bad file: tests/missing.yaml"
set_tests_properties(
  missing_yaml PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: bad file: .*/missing.yaml"
)

# Test for missing "name" field
add_test(
  NAME missing_name
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/missing_name.yaml
)

# Mark test as expected to fail with "Error: Field name is required"
set_tests_properties(
  missing_name PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field name is required"
)

# Test for missing "elf_file" field
add_test(
  NAME missing_elf_file
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/missing_elf_file.yaml
)

# Mark test as expected to fail with "Error: Field elf_file is required"
set_tests_properties(
  missing_elf_file PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field elf_file is required"
)

# Test for missing "iteration_count" field
add_test(
  NAME missing_iteration_count
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/missing_iteration_count.yaml
)

# Mark test as expected to fail with "Error: Field iteration_count is required"
set_tests_properties(
  missing_iteration_count PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field iteration_count is required"
)

# Test for missing "program_cpu_assignment" field
add_test(
  NAME missing_program_cpu_assignment
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/missing_program_cpu_assignment.yaml
)

# Mark test as expected to fail with "Error: Field program_cpu_assignment is required"
set_tests_properties(
  missing_program_cpu_assignment PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field program_cpu_assignment is required"
)

# Test for empty "program_cpu_assignment" field
add_test(
  NAME empty_program_cpu_assignment
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/empty_program_cpu_assignment.yaml
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
#   COMMAND sudo bin/bpf_performance_runner ${TEST_FILE_DIRECTORY}/elf_file_not_found.yaml
# )

# # Mark test as expected to fail with "libbpf: elf: failed to open not_a_file.o: No such file or directory"
# set_tests_properties(
#   elf_file_not_found PROPERTIES
#   PASS_REGULAR_EXPRESSION "libbpf: elf: failed to open not_a_file.o: No such file or directory"
# )

# Test for function map_state_preparation not found
add_test(
  NAME function_map_state_preparation_not_found
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/function_map_state_preparation_not_found.yaml
)

# Mark test as expected to fail with "Error: Failed to find map_state_preparation program not_a_real_function"
set_tests_properties(
  function_map_state_preparation_not_found PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Failed to find map_state_preparation program not_a_real_function"
)

# Test for map_state_preparation.program not found
add_test(
  NAME map_state_preparation_program_not_found
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/map_state_preparation_program_not_found.yaml
)

# Mark test as expected to fail with "Error: Field map_state_preparation.program is required"
set_tests_properties(
  map_state_preparation_program_not_found PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field map_state_preparation.program is required"
)

# Test for map_state_preparation.iteration_count not found
add_test(
  NAME map_state_preparation_iteration_count_not_found
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/map_state_preparation_iteration_count_not_found.yaml
)

# Mark test as expected to fail with "Error: Field map_state_preparation.iteration_count is required"
set_tests_properties(
  map_state_preparation_iteration_count_not_found PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Field map_state_preparation.iteration_count is required"
)

# Test for program_cpu_assignment.program not found
add_test(
  NAME function_program_cpu_assignment_not_found
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/function_program_cpu_assignment_not_found.yaml
)

# Mark test as expected to fail with "Error: Failed to find program not_a_function"
set_tests_properties(
  function_program_cpu_assignment_not_found PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Failed to find program not_a_function"
)

# Test for invalid program_cpu_assignment value
add_test(
  NAME invalid_program_cpu_assignment_value
  COMMAND sudo bin/bpf_performance_runner -i ${TEST_FILE_DIRECTORY}/invalid_program_cpu_assignment_value.yaml
)

# Mark test as expected to fail with "Error: Invalid program_cpu_assignment - must be string or sequence"
set_tests_properties(
  invalid_program_cpu_assignment_value PROPERTIES
  PASS_REGULAR_EXPRESSION "Error: Invalid program_cpu_assignment - must be string or sequence"
)