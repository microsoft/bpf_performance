// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

// This program loads an EBPF program and runs it via bpf_prog_test_run_opts on
// one or more CPU cores and then reports the average duration on each CPU.

#include <cstring>
#include <iostream>
#include <optional>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <linux/bpf.h>

// This is a work around for bpf_stats_type enum not being defined in
// linux/bpf.h. This enum is defined in bpf.h in the kernel source tree.
#define bpf_stats_type bpf_stats_type_fake
enum bpf_stats_type_fake
{
};
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#undef bpf_stats_type_fake

/**
 * @brief Read in a string of hex bytes and return a vector of bytes.
 *
 * @param[in] input String containing hex bytes.
 * @return Vector of bytes.
 */
std::vector<uint8_t>
base16_decode(const std::string &input)
{
    std::vector<uint8_t> output;
    std::stringstream ss(input);
    std::string value;
    while (std::getline(ss, value, ' '))
    {
        try
        {
            output.push_back(std::stoi(value, nullptr, 16));
        }
        catch (...)
        {
            // Ignore invalid values.
        }
    }
    return output;
}

int load_elf_file(const std::string &program_string, const std::optional<std::string> &program_name)
{
    int fd = -1;
    struct bpf_object *obj = nullptr;
    struct bpf_program *prog = nullptr;
    struct bpf_map *map = nullptr;
    int err = 0;

    object = bpf_object__open(program_string.c_str());
    if (!object)
    {
        throw std::runtime_error("Failed to open object" + program_string);
    }

    if (program_name.has_value())
    {
        program = bpf_object__find_program_by_name(object, program_name.value().c_str());
        if (!program)
        {
            throw std::runtime_error("Failed to find program");
        }
    }
    else
    {
        program = bpf_object__next_program(object, nullptr);
        if (!program)
        {
            throw std::runtime_error("Failed to find program");
        }
    }

    return bpf_program__fd(program);
}

int main(int argc, char **argv)
{
    try
    {
        std::string program_name;
        std::string input_data_file;
        std::string input_context_file;
        std::vector<uint8_t> input_data;
        std::vector<uint8_t> input_context;
        size_t cpu_count = 1;
        size_t repeat_count = 1;
        size_t batch_size = 1;
        std::vector<std::string> args(argv, argv + argc);
        if (args.size() > 0)
        {
            args.erase(args.begin());
        }

        if (args.size() > 0 && args[0] == "--help")
        {
            std::cout << "usage: " << argv[0] << " --program <bpf_program_to_test> [--cpu_count <number of CPU>] [--repeat_count <number of iterations>] [--batch_size <batch size>] [--data_in <file for data_in>] [--context_in <file for context input]" << std::endl;
            return 1;
        }

        while (args.size() > 0)
        {
            if (args[0] == "--program" && args.size() > 1)
            {
                program_name = args[1];
                args.erase(args.begin(), args.begin() + 2);
            }
            else if (args[0] == "--cpu_count" && args.size() > 1)
            {
                cpu_count = std::stoi(args[1]);
                args.erase(args.begin(), args.begin() + 2);
            }
            else if (args[0] == "--repeat_count" && args.size() > 1)
            {
                repeat_count = std::stoi(args[1]);
                args.erase(args.begin(), args.begin() + 2);
            }
            else if (args[0] == "--batch_size" && args.size() > 1)
            {
                batch_size = std::stoi(args[1]);
                args.erase(args.begin(), args.begin() + 2);
            }
            else if (args[0] == "--data_in" && args.size() > 1)
            {
                input_data_file = args[1];
                args.erase(args.begin(), args.begin() + 2);
            }
            else if (args[0] == "--context_in" && args.size() > 1)
            {
                input_context_file = args[1];
                args.erase(args.begin(), args.begin() + 2);
            }
            else
            {
                std::cout << "Invalid argument: " << args[0] << std::endl;
                return 1;
            }
        }

        if (program_name.empty())
        {
            std::cout << "Missing program name" << std::endl;
            return 1;
        }

        if (!input_data_file.empty())
        {
            std::ifstream file(input_data_file);
            if (!file.is_open())
            {
                std::cout << "Failed to open data file: " << input_data_file << std::endl;
                return 1;
            }
            std::string data;
            std::getline(file, data);
            input_data = base16_decode(data);
        }

        if (!input_context_file.empty())
        {
            std::ifstream file(input_context_file);
            if (!file.is_open())
            {
                std::cout << "Failed to open context file: " << input_context_file << std::endl;
                return 1;
            }
            std::string data;
            std::getline(file, data);
            input_context = base16_decode(data);
        }

        fd = load_elf_file(program_string, log);

        if (fd < 0)
        {
            return 1;
        }

        std::vector<std::thread> threads;

        for (size_t cpu = 0; < cpu_count; cpu++)
        {
            threads.emplace_back([cpu, fd, repeat_count, batch_size, input_data, input_context]()
                                 {
            // Run program.
            bpf_test_run_opts test_run{
                .sz = sizeof(bpf_test_run_opts),
                .ctx_in = input_context.data(),
                .ctx_size_in = static_cast<uint32_t>(input_context.size()),
                .data_in = input_data.data(),
                .data_size_in = static_cast<uint32_t>(input_data.size()),
                .repeat = repeat_count,
                .cpu = cpu,
                .batch_size = batch_size,
            };
            int result = bpf_prog_test_run_opts(fd, &test_run);
            if (result == 0)
            {
                // Print output.
                std::cout << "CPU " << cpu << ": " << test_run.duration << "ns" << std::endl;
            } });
        }
        for (auto &thread : threads)
        {
            thread.join();
        }

        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
