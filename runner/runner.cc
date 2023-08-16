// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <iostream>
#include <optional>
#include <regex>
#include <thread>
#include <vector>
#include <yaml-cpp/yaml.h>

// Define unique_ptr to call bpf_object__close on destruction
struct bpf_object_deleter
{
    void
    operator()(struct bpf_object* obj) const
    {
        if (obj) {
            bpf_object__close(obj);
        }
    }
};

typedef std::unique_ptr<struct bpf_object, bpf_object_deleter> bpf_object_ptr;

// Set string runner_platform to "linux" to indicate that this is a Linux runner.
#if defined(__linux__)
const std::string runner_platform = "Linux";
#else
const std::string runner_platform = "Windows";
#endif


// This program runs a set of BPF programs and reports the average execution time for each program.
// It reads a YAML file that contains the following fields:
// - tests: a list of tests to run
//   - name: the name of the test
//   - elf_file: the path to the BPF object file
//   - iteration_count: the number of times to run each program
//   - map_state_preparation: optional, a program to run before the test to prepare the map state
//     - program: the name of the program
//     - iteration_count: the number of times to run the program
//   - program_cpu_assignment: a map of program names to CPUs
//     - <program name>: the name of the program
//       - <cpu number>: the CPU number to run the program on
//       - all: run the program on all CPUs
//       - unassigned: run the program on all unassigned CPUs
int
main(int argc, char** argv)
{
    try {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <config.yaml> [test]" << std::endl;
            return 1;
        }

        // Get the optional test name.
        std::optional<std::string> test_name;
        if (argc > 2) {
            test_name = argv[2];
        }

        YAML::Node config = YAML::LoadFile(argv[1]);
        auto tests = config["tests"];
        std::map<std::string, bpf_object_ptr> bpf_objects;

        // Query libbpf for cpu count.
        int cpu_count = libbpf_num_possible_cpus();

        // Fail if tests is empty or not a sequence.
        if (!tests || !tests.IsSequence()) {
            throw std::runtime_error("Invalid config file - tests must be a sequence");
        }

        // Run each test.
        for (auto test : tests) {
            // Check for required fields.
            if (!test["name"].IsDefined()) {
                throw std::runtime_error("Field name is required");
            }

            if (!test["elf_file"].IsDefined()) {
                throw std::runtime_error("Field elf_file is required");
            }

            if (!test["iteration_count"].IsDefined()) {
                throw std::runtime_error("Field iteration_count is required");
            }

            if (!test["program_cpu_assignment"].IsDefined()) {
                throw std::runtime_error("Field program_cpu_assignment is required");
            }

            if (!test["program_cpu_assignment"].IsMap()) {
                throw std::runtime_error("Field program_cpu_assignment must be a map");
            }

            std::string name = test["name"].as<std::string>();
            std::string elf_file = test["elf_file"].as<std::string>();
            int iteration_count = test["iteration_count"].as<int>();

            // Check if value "platform" is defined and matches the current platform.
            if (test["platform"].IsDefined()) {
                std::string platform = test["platform"].as<std::string>();
                if (runner_platform != platform) {
                    // Don't run this test if the platform doesn't match.
                    continue;
                }
            }

            // Skip if test name is specified and doesn't match, with test name being a regex.
            if (test_name && !std::regex_match(name, std::regex(*test_name))) {
                continue;
            }

            if (bpf_objects.find(elf_file) == bpf_objects.end()) {
                bpf_object_ptr obj;
                obj.reset(bpf_object__open(elf_file.c_str()));
                if (!obj) {
                    throw std::runtime_error("Failed to open BPF object " + elf_file + ": " + strerror(errno));
                }
                if (bpf_object__load(obj.get()) < 0) {
                    throw std::runtime_error("Failed to load BPF object " + elf_file + ": " + strerror(errno));
                }

                // Insert into bpf_objects
                bpf_objects.insert({elf_file, std::move(obj)});
            }

            // Vector of CPU -> program fd.
            std::vector<std::optional<int>> cpu_program_assignments(cpu_count);

            bpf_object_ptr& obj = bpf_objects[elf_file];

            // Check if node map_state_preparation exits.
            auto map_state_preparation = test["map_state_preparation"];
            if (map_state_preparation) {
                if (!map_state_preparation["program"].IsDefined()) {
                    throw std::runtime_error("Field map_state_preparation.program is required");
                }

                if (!map_state_preparation["iteration_count"].IsDefined()) {
                    throw std::runtime_error("Field map_state_preparation.iteration_count is required");
                }

                std::string prep_program_name = map_state_preparation["program"].as<std::string>();
                int prep_program_iterations = map_state_preparation["iteration_count"].as<int>();
                auto map_state_preparation_program =
                    bpf_object__find_program_by_name(obj.get(), prep_program_name.c_str());
                if (!map_state_preparation_program) {
                    throw std::runtime_error("Failed to find map_state_preparation program " + prep_program_name);
                }

                // Run map_state_preparation program via bpf_prog_test_run_opts.
                std::vector<uint8_t> data_in(1024);
                std::vector<uint8_t> data_out(1024);
                std::vector<uint8_t> context_in(4);
                std::vector<uint8_t> context_out(4);

                bpf_test_run_opts opts;
                memset(&opts, 0, sizeof(opts));
                opts.sz = sizeof(opts);
                opts.repeat = prep_program_iterations;
                opts.data_in = data_in.data();
                opts.data_out = data_out.data();
                opts.data_size_in = static_cast<uint32_t>(data_in.size());
                opts.data_size_out = static_cast<uint32_t>(data_out.size());

                if (bpf_prog_test_run_opts(bpf_program__fd(map_state_preparation_program), &opts)) {
                    throw std::runtime_error("Failed to run map_state_preparation program " + prep_program_name);
                }

                if (opts.retval != 0) {
                    throw std::runtime_error(
                        "map_state_preparation program " + prep_program_name + " returned non-zero");
                }
            }

            auto program_cpu_assignment = test["program_cpu_assignment"];
            for (auto assignment : program_cpu_assignment) {
                // Each node is a program name and a cpu number or a list of cpu numbers.
                // First check if program exists and get program fd.

                auto program_name = assignment.first.as<std::string>();
                auto program = bpf_object__find_program_by_name(obj.get(), program_name.c_str());
                if (!program) {
                    throw std::runtime_error("Failed to find program " + program_name);
                }

                int program_fd = bpf_program__fd(program);

                // Check if assignment is scalar or sequence
                if (assignment.second.IsScalar()) {
                    if (assignment.second.as<std::string>() == "all") {
                        // Assign program to all CPUs.
                        for (size_t i = 0; i < cpu_program_assignments.size(); i++) {
                            cpu_program_assignments[i] = {program_fd};
                        }
                    } else if (assignment.second.as<std::string>() == "unassigned") {
                        // Assign program to all unassigned CPUs.
                        for (size_t i = 0; i < cpu_program_assignments.size(); i++) {
                            if (!cpu_program_assignments[i].has_value()) {
                                cpu_program_assignments[i] = {program_fd};
                            }
                        }
                    } else {
                        int cpu = assignment.second.as<int>();
                        if (cpu > cpu_count) {
                            throw std::runtime_error("Invalid CPU number " + std::to_string(cpu));
                        }
                        cpu_program_assignments[assignment.as<int>()] = {program_fd};
                    }
                } else if (assignment.second.IsSequence()) {
                    for (auto cpu_assignment : assignment.second) {
                        int cpu = cpu_assignment.as<int>();
                        if (cpu > cpu_count) {
                            throw std::runtime_error("Invalid CPU number " + std::to_string(cpu));
                        }
                        cpu_program_assignments[cpu] = {program_fd};
                    }
                } else {
                    throw std::runtime_error("Invalid program_cpu_assignment - must be string or sequence");
                }
            }

            // Run each entry point via bpf_prog_test_run_opts in a thread.
            std::vector<std::jthread> threads;
            std::vector<bpf_test_run_opts> opts(cpu_count);

            for (size_t i = 0; i < cpu_program_assignments.size(); i++) {
                if (!cpu_program_assignments[i].has_value()) {
                    continue;
                }
                auto program = cpu_program_assignments[i].value();
                auto& opt = opts[i];

                threads.emplace_back([=, &opt](std::stop_token stop_token) {
                    memset(&opt, 0, sizeof(opt));
                    std::vector<uint8_t> data_in(1024);
                    std::vector<uint8_t> data_out(1024);

                    opt.sz = sizeof(opt);
                    opt.repeat = iteration_count;
                    opt.cpu = i;
                    opt.data_in = data_in.data();
                    opt.data_out = data_out.data();
                    opt.data_size_in = static_cast<uint32_t>(data_in.size());
                    opt.data_size_out = static_cast<uint32_t>(data_out.size());
                    #if defined(HAS_BPF_TEST_RUN_OPTS_BATCH_SIZE)
                    opt.batch_size = 64;
                    #endif

                    int result = bpf_prog_test_run_opts(program, &opt);
                    if (result < 0) {
                        opt.retval = result;
                    }
                });
            }
            for (auto& thread : threads) {
                thread.join();
            }

            // Check if any program returned non-zero.
            for (auto& opt : opts) {
                if (opt.retval != 0) {
                    throw std::runtime_error("Program returned non-zero "+ std::to_string(opt.retval) +" in test " + name);
                }
            }

            // Print the average execution time for each program on each CPU.
            std::cout << name << ",";

            for (size_t i = 0; i < opts.size(); i++) {
                if (!cpu_program_assignments[i].has_value()) {
                    continue;
                }
                auto& opt = opts[i];
                std::cout << opt.duration;
                if (i < opts.size() - 1) {
                    std::cout << ",";
                }
            }
            std::cout << std::endl;
        }

        return 0;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}