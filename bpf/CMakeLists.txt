# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Each test consists of a C file, an output file name, and an optional option, seperated by commas.
set(test_cases
    "baseline"
    "generic_map,hash,-DTYPE=BPF_MAP_TYPE_HASH"
    "generic_map,percpu_hash,-DTYPE=BPF_MAP_TYPE_PERCPU_HASH"
    "generic_map,lru_hash,-DTYPE=BPF_MAP_TYPE_LRU_HASH"
    "generic_map,lru_per_cpu_hash,-DTYPE=BPF_MAP_TYPE_LRU_PERCPU_HASH"
    "generic_map,array,-DTYPE=BPF_MAP_TYPE_ARRAY"
    "generic_map,percpu_array,-DTYPE=BPF_MAP_TYPE_PERCPU_ARRAY"
    "helpers"
    "lpm,lpm_1024,-DMAX_ENTRIES=1024"
    "lpm,lpm_16384,-DMAX_ENTRIES=16384"
    "lpm,lpm_262144,-DMAX_ENTRIES=262144"
    "lpm,lpm_1048576,-DMAX_ENTRIES=1048576"
    "map_in_map,hash_of_array,-DTYPE=BPF_MAP_TYPE_HASH_OF_MAPS"
    "map_in_map,array_of_array,-DTYPE=BPF_MAP_TYPE_ARRAY_OF_MAPS"
    "ringbuf"
    "rolling_lru"
    "tail_call"
    "xdp"
    "max_tail_call"
    )

function(process_test_cases worker)
    foreach(test ${test_cases})
        # Split test into list of strings
        string(REPLACE "," " " elements ${test})
        separate_arguments(elements NATIVE_COMMAND ${elements})

        # Append out_file and options if they are missing
        list(LENGTH elements num_elements)

        if (num_elements EQUAL 1)
            list(GET elements 0 file_name)
            set(out_name ${file_name})
            set(option "-DBPF")
        endif()
        if (num_elements EQUAL 2)
            list(GET elements 0 file_name)
            list(GET elements 1 out_name)
            set(option "-DBPF")
        endif()
        if (num_elements EQUAL 3)
            list(GET elements 0 file_name)
            list(GET elements 1 out_name)
            list(GET elements 2 option)
        endif()

        cmake_language(CALL ${worker} ${file_name} ${out_name} ${option})
    endforeach()
endfunction()

find_program(clang_path "clang")

function(build_bpf file_name out_name option)
    message(STATUS "Building BPF ${out_name}")

    set(optimize_flags "-O2")
    set(option_flags)

    set(bpf_file_name ${file_name}.c)
    set(bpf_file_path ${CMAKE_CURRENT_SOURCE_DIR}/${bpf_file_name})
    set(bpf_obj_file_name ${out_name}.o)
    set(bpf_obj_file_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${bpf_obj_file_name})

    if (NOT EXISTS ${bpf_file_path})
        message(FATAL_ERROR "BPF file ${bpf_file_path} does not exist")
    endif()

    add_custom_command(
        OUTPUT ${bpf_obj_file_path}
        COMMAND ${clang_path} ${option} -I ${EBPF_INC_PATH} -I ${CMAKE_CURRENT_BINARY_DIR} -g ${optimize_flags} -target bpf -c ${bpf_file_path} -o ${bpf_obj_file_path}
        DEPENDS ${bpf_file_path}
        COMMENT "Building BPF object ${bpf_obj_file_path}"
    )

    add_custom_target(${out_name}_ELF ALL DEPENDS ${bpf_obj_file_path} SOURCES ${bpf_file_path})
endfunction()

# Run packages\eBPF-for-Windows\build\native\bin\Convert-BpfToNative.ps1 on each .o file to produce a .sys file
function(convert_to_native file_name out_name option)
    # Run export_program_info.exe as a pre-build step to export the program information.
    # This is needed to convert the .o file to a .sys file.
    add_custom_command(
        OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/export_program_info.log
        COMMAND ${EBPF_BIN_PATH}/export_program_info.exe > ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/export_program_info.log
        DEPENDS ${EBPF_BIN_PATH}/export_program_info.exe
        COMMENT "Exporting program information"
        PRE_BUILD
    )
    message(STATUS "Converting BPF ${out_name} to native")

    set(bpf_obj_file_name ${out_name}.o)
    set(bpf_obj_file_path ${CMAKE_CURRENT_BINARY_DIR}/${bpf_obj_file_name})
    set(bpf_sys_file_name ${out_name}.sys)
    set(bpf_pdb_file_name ${out_name}.pdb)
    set(bpf_sys_file_path ${CMAKE_CURRENT_BINARY_DIR}/${bpf_sys_file_name})
    set(bpf_pdb_file_path ${CMAKE_CURRENT_BINARY_DIR}/${bpf_pdb_file_name})
    set(bpf_sys_file_output_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${bpf_sys_file_name})
    set(bpf_pdb_file_output_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${bpf_pdb_file_name})

    # Copy the .o file to the output directory as part of post build
    add_custom_command(
        OUTPUT ${bpf_obj_file_path}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${bpf_obj_file_name} ${bpf_obj_file_path}
        DEPENDS ${bpf_obj_file_name}
        COMMENT "Copying BPF object ${bpf_obj_file_path} to output directory"
        POST_BUILD)

    # Run the powershell script to convert the .o file to a .sys file
    add_custom_command(
        OUTPUT ${bpf_sys_file_path} ${bpf_pdb_file_path}
        COMMAND powershell -ExecutionPolicy Bypass -File ${EBPF_BIN_PATH}/Convert-BpfToNative.ps1 -FileName ${bpf_obj_file_name} -IncludeDir ${EBPF_INC_PATH} -OutDir ${CMAKE_CURRENT_BINARY_DIR} -BinDir ${EBPF_BIN_PATH} -Configuration $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>
        DEPENDS ${bpf_obj_file_path} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/export_program_info.log
        COMMENT "Converting BPF object ${bpf_obj_file_path} to native"
        POST_BUILD
    )

    # Add a custom target that depends on the .sys file
    add_custom_target(${out_name}_SYS ALL DEPENDS ${bpf_sys_file_path} ${bpf_pdb_file_path} SOURCES ${bpf_obj_file_path})

    # Copy the .sys file to the output directory as part of post build
    add_custom_command(
        OUTPUT ${bpf_sys_file_output_path}
        COMMAND ${CMAKE_COMMAND} -E copy ${bpf_sys_file_path} ${bpf_sys_file_output_path}
        DEPENDS ${bpf_sys_file_path}
        COMMENT "Copying BPF object ${bpf_sys_file_path} to output directory"
        POST_BUILD)

    # Copy the .pdb file to the output directory as part of post build
    add_custom_command(
        OUTPUT ${bpf_pdb_file_output_path}
        COMMAND ${CMAKE_COMMAND} -E copy ${bpf_pdb_file_path} ${bpf_pdb_file_output_path}
        DEPENDS ${bpf_pdb_file_path}
        COMMENT "Copying BPF object ${bpf_pdb_file_path} to output directory"
        POST_BUILD)

    # Add a custom target that depends on the .sys file
    add_custom_target(${bpf_sys_file_name}_out ALL DEPENDS ${bpf_sys_file_output_path} SOURCES ${bpf_sys_file_path})

    # Add a custom target that depends on the .pdb file
    add_custom_target(${bpf_pdb_file_name}_out ALL DEPENDS ${bpf_pdb_file_output_path} SOURCES ${bpf_pdb_file_path})
endfunction()

configure_file(
    bpf.h.inc
    ${CMAKE_CURRENT_BINARY_DIR}/bpf.h
)

process_test_cases("build_bpf")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/tests.yml
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests.yml COPYONLY)

if (PLATFORM_WINDOWS)
    process_test_cases("convert_to_native")
endif()