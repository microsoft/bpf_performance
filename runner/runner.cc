// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include <iostream>
#include <thread>
#include <vector>
#include <optional>
#include <yaml-cpp/yaml.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>

// Define unique_ptr to call bpf_object__close on destruction
struct bpf_object_deleter
{
    void operator()(struct bpf_object *obj) const
    {
        if (obj)
        {
            bpf_object__close(obj);
        }
    }
};

typedef std::unique_ptr<struct bpf_object, bpf_object_deleter> bpf_object_ptr;

int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <config.yaml>" << std::endl;
            return 1;
        }

        YAML::Node config = YAML::LoadFile(argv[1]);
        auto tests = config["tests"];

        // Query libbpf for cpu count.
        int cpu_count = libbpf_num_possible_cpus();

        // Run each test.
        for (auto test : tests)
        {
            // Check for required fields.
            if (test["name"].IsNull())
            {
                throw std::runtime_error("Test name is required");
            }

            if (test["elf_file"].IsNull())
            {
                throw std::runtime_error("Test elf_file is required");
            }

            if (test["iteration_count"].IsNull())
            {
                throw std::runtime_error("Test iteration_count is required");
            }

            std::string name = test["name"].as<std::string>();
            std::string elf_file = test["elf_file"].as<std::string>();
            int iteration_count = test["iteration_count"].as<int>();

            // Vector of program names to run.
            std::vector<std::string> entry_points;

            // Vector of CPU -> program fd.
            std::vector<std::optional<int>> cpu_program_assignments(cpu_count);

            bpf_object_ptr obj;
            obj.reset(bpf_object__open(elf_file.c_str()));
            if (!obj)
            {
                throw std::runtime_error("Failed to open BPF object " + elf_file + ": " + strerror(errno));
            }
            if (bpf_object__load(obj.get()) < 0)
            {
                throw std::runtime_error("Failed to load BPF object " + elf_file + ": " + strerror(errno));
            }

            // Check if node map_state_preparation exits.
            auto map_state_preparation = test["map_state_preparation"];
            if (map_state_preparation)
            {
                std::string prep_program_name = map_state_preparation["program"].as<std::string>();
                int prep_program_iterations = map_state_preparation["iteration_count"].as<int>();
                auto map_state_preparation_program = bpf_object__find_program_by_name(obj.get(), prep_program_name.c_str());
                if (!map_state_preparation_program)
                {
                    throw std::runtime_error("Failed to find map_state_preparation program " + prep_program_name);
                }

                // Run map_state_preparation program via bpf_prog_test_run_opts.
                bpf_test_run_opts opts;
                memset(&opts, 0, sizeof(opts));
                opts.sz = sizeof(opts);
                opts.repeat = prep_program_iterations;

                if (bpf_prog_test_run_opts(bpf_program__fd(map_state_preparation_program), &opts))
                {
                    throw std::runtime_error("Failed to run map_state_preparation program " + prep_program_name);
                }

                if (opts.retval != 0)
                {
                    throw std::runtime_error("map_state_preparation program " + prep_program_name + " returned non-zero");
                }
            }

            auto program_cpu_assignment = test["program_cpu_assignment"];
            if (program_cpu_assignment)
            {
                for (auto assignment : program_cpu_assignment)
                {
                    // Each node is a program name and a cpu number or a list of cpu numbers.
                    // First check if program exists and get program fd.

                    auto program_name = assignment.first.as<std::string>();
                    auto program = bpf_object__find_program_by_name(obj.get(), program_name.c_str());
                    if (!program)
                    {
                        throw std::runtime_error("Failed to find program " + program_name);
                    }

                    int program_fd = bpf_program__fd(program);

                    // Check if assignment is scalar or sequence
                    if (assignment.second.IsScalar())
                    {
                        if (assignment.second.as<std::string>() == "all")
                        {
                            // Assign program to all CPUs.
                            for (size_t i = 0; i < cpu_program_assignments.size(); i++)
                            {
                                cpu_program_assignments[i] = {program_fd};
                            }
                        }
                        else if (assignment.second.as<std::string>() == "unassigned")
                        {
                            // Assign program to all unassigned CPUs.
                            for (size_t i = 0; i < cpu_program_assignments.size(); i++)
                            {
                                if (!cpu_program_assignments[i].has_value())
                                {
                                    cpu_program_assignments[i] = {program_fd};
                                }
                            }
                        }
                        else
                        {
                            int cpu = assignment.second.as<int>();
                            if (cpu > cpu_count)
                            {
                                throw std::runtime_error("Invalid CPU number " + std::to_string(cpu));
                            }
                            cpu_program_assignments[assignment.as<int>()] = {program_fd};
                        }
                    }
                    else if (assignment.second.IsSequence())
                    {
                        for (auto cpu_assignment : assignment.second)
                        {
                            int cpu = cpu_assignment.as<int>();
                            if (cpu > cpu_count)
                            {
                                throw std::runtime_error("Invalid CPU number " + std::to_string(cpu));
                            }
                            cpu_program_assignments[cpu] = {program_fd};
                        }
                    }
                    else
                    {
                        throw std::runtime_error("Invalid program_cpu_assignment - must be string or sequence");
                    }
                }
            }
            else
            {
                // Assign each program to a CPU in round-robin fashion.
                int cpu = 0;
                for (auto &program : entry_points)
                {
                    auto program_fd = bpf_object__find_program_by_name(obj.get(), program.c_str());
                    if (!program_fd)
                    {
                        throw std::runtime_error("Failed to find program " + program);
                    }
                    cpu_program_assignments[cpu] = {bpf_program__fd(program_fd)};
                    cpu = (cpu + 1) % cpu_count;
                }
            }

            // Run each entry point via bpf_prog_test_run_opts in a thread.
            std::vector<std::jthread> threads;
            std::vector<bpf_test_run_opts> opts(cpu_count);

            for (size_t i = 0; i < cpu_program_assignments.size(); i++)
            {
                if (!cpu_program_assignments[i].has_value())
                {
                    continue;
                }
                auto program = cpu_program_assignments[i].value();
                auto &opt = opts[i];
                memset(&opt, 0, sizeof(opt));
                opt.sz = sizeof(opt);
                opt.repeat = iteration_count;
                opt.cpu = i;

                threads.emplace_back([program, &opt](std::stop_token stop_token)
                                     {
                int result = bpf_prog_test_run_opts(program, &opt);
                if (result < 0) {
                    opt.retval = result;
                } });
            }
            for (auto &thread : threads)
            {
                thread.join();
            }

            // Check if any program returned non-zero.
            for (auto &opt : opts)
            {
                if (opt.retval != 0)
                {
                    throw std::runtime_error("Program returned non-zero");
                }
            }

            // Print the average execution time for each program on each CPU.
            std::cout << name << ",";

            for (size_t i = 0; i < opts.size(); i++)
            {
                if (!cpu_program_assignments[i].has_value())
                {
                    continue;
                }
                auto &opt = opts[i];
                std::cout << opt.duration;
                if (i < opts.size() - 1)
                {
                    std::cout << ",";
                }
            }
            std::cout << std::endl;
        }

        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}