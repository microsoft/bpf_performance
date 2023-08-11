# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

find_program(clang_path "clang")

function(build_bpf file_name)
    message(STATUS "Building BPF ${file_name}")

    set(optimize_flags "-O2")

    set(bpf_file_name ${file_name}.c)
    set(bpf_file_path ${CMAKE_CURRENT_SOURCE_DIR}/${bpf_file_name})
    set(bpf_obj_file_name ${file_name}.o)
    set(bpf_obj_file_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${bpf_obj_file_name})

    if (NOT EXISTS ${bpf_file_path})
        message(FATAL_ERROR "BPF file ${bpf_file_path} does not exist")
    endif()

    add_custom_command(
        OUTPUT ${bpf_obj_file_path}
        COMMAND ${clang_path} -I ${EBPF_INC_PATH} -I ${CMAKE_CURRENT_BINARY_DIR} -g ${optimize_flags} -target bpf -c ${bpf_file_path} -o ${bpf_obj_file_path}
        DEPENDS ${bpf_file_path}
        COMMENT "Building BPF object ${bpf_obj_file_path}"
    )

    add_custom_target(${file_name}_ELF ALL DEPENDS ${bpf_obj_file_path} SOURCES ${bpf_file_path})
endfunction()

configure_file(
    bpf.h.inc
    ${CMAKE_CURRENT_BINARY_DIR}/bpf.h
)

build_bpf(baseline)
build_bpf(generic_map)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests.yml DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})