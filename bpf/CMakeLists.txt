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

# Run the build_bpf function on each .c file
file(GLOB bpf_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)

foreach(bpf_file ${bpf_files})
    string(REPLACE ".c" "" file_name ${bpf_file})
    build_bpf(${file_name})
endforeach()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests.yml DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if (PLATFORM_WINDOWS)
    # Run export_program_info.exe as a pre-build step to export the program information.
    # This is needed to convert the .o file to a .sys file.
    add_custom_command(
        OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/export_program_info.log
        COMMAND ${EBPF_BIN_PATH}/export_program_info.exe > ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/export_program_info.log
        DEPENDS ${EBPF_BIN_PATH}/export_program_info.exe
        COMMENT "Exporting program information"
        PRE_BUILD
    )

    # Run packages\eBPF-for-Windows\build\native\bin\Convert-BpfToNative.ps1 on each .o file to produce a .sys file
    function(convert_to_native file_name)
        message(STATUS "Converting BPF ${file_name} to native")

        set(bpf_obj_file_name ${file_name}.o)
        set(bpf_obj_file_path ${CMAKE_CURRENT_BINARY_DIR}/${bpf_obj_file_name})
        set(bpf_sys_file_name ${file_name}.sys)
        set(bpf_sys_file_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${bpf_sys_file_name})

        # Copy the .o file to the output directory as part of post build
        add_custom_command(
            OUTPUT ${bpf_obj_file_path}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${bpf_obj_file_name} ${bpf_obj_file_path}
            DEPENDS ${bpf_obj_file_name}
            COMMENT "Copying BPF object ${bpf_obj_file_path} to output directory"
            POST_BUILD)

        # Run the powershell script to convert the .o file to a .sys file
        add_custom_command(
            OUTPUT ${bpf_sys_file_path}
            COMMAND powershell -ExecutionPolicy Bypass -File ${EBPF_BIN_PATH}/Convert-BpfToNative.ps1 -FileName ${bpf_obj_file_name} -IncludeDir ${EBPF_INC_PATH} -OutDir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} -BinDir ${EBPF_BIN_PATH}
            DEPENDS ${bpf_obj_file_path} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/export_program_info.log
            COMMENT "Converting BPF object ${bpf_obj_file_path} to native"
            POST_BUILD
        )

        # Add a custom target that depends on the .sys file
        add_custom_target(${file_name}_SYS ALL DEPENDS ${bpf_sys_file_path} SOURCES ${bpf_obj_file_path})
    endfunction()

    # Run the convert_to_native function on each .o file

    foreach(bpf_file ${bpf_files})
        string(REPLACE ".c" "" file_name ${bpf_file})
        convert_to_native(${file_name})
    endforeach()
endif()