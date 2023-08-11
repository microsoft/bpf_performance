# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

function(clang_validator correct clang_path)
    execute_process(
        COMMAND echo "int main() { return 0;}"
        COMMAND ${clang_path} --target=bpf -x c - -c -o /dev/null
        ERROR_QUIET OUTPUT_QUIET
        RESULT_VARIABLE CLANG_RETURN_CODE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (CLANG_RETURN_CODE EQUAL 0)
        set(${correct} true PARENT_SCOPE)
    else()
        set(${correct} false PARENT_SCOPE)
    endif()

endfunction()

find_program(clang_path "clang" VALIDATOR clang_validator NO_CACHE HINTS ${BPF_PERF_ALTERNATE_LLVM_PATH})

function(build_bpf file_name)
    message(STATUS "Building BPF ${file_name}")

    set(optimize_flags "-O2")

    set(bpf_file_name ${file_name}.c)
    set(bpf_file_path ${CMAKE_CURRENT_SOURCE_DIR}/${bpf_file_name})
    set(bpf_obj_file_name ${file_name}.o)
    set(bpf_obj_file_path ${CMAKE_CURRENT_BINARY_DIR}/${bpf_obj_file_name})

    if (NOT EXISTS ${bpf_file_path})
        message(FATAL_ERROR "BPF file ${bpf_file_path} does not exist")
    endif()

    add_custom_command(
        OUTPUT ${bpf_obj_file_path}
        COMMAND ${clang_path} -g ${optimize_flags} -target bpf -c ${bpf_file_path} -o ${bpf_obj_file_path}
        DEPENDS ${bpf_file_path}
        COMMENT "Building BPF object ${bpf_obj_file_path}"
    )

    add_custom_target(${file_name}_ELF ALL DEPENDS ${bpf_obj_file_path} SOURCES ${bpf_file_path})
endfunction()

build_bpf(baseline)
build_bpf(generic_map)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests.yml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})