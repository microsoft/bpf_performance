// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include "options.h"

void options::parse(int argc, char **argv) {
  args = {argv, argv + argc};
  auto iter = args.begin();
  iter++;

  while (iter != args.end()) {
    std::string arg = *iter;

    auto it = option_map.find(arg);
    if (it == option_map.end()) {
      out << "Unknown option: " << arg << std::endl;
      print_help();
      exit(1);
    }

    const auto &[num_args, func, help] = it->second;

    if (iter - args.begin() + num_args > args.size()) {
      out << "Option " << arg << " requires " << num_args << " arguments"
          << std::endl;
      print_help();
      exit(1);
    }

    func(iter + 1);
    iter += num_args;
  }
}

void options::add(const std::string &option, size_t num_args,
                  option_handler func, const std::string &help) {
  option_map[option] = std::make_tuple(num_args, func, help);
}

void options::print_help() {
  out << "Usage: " << args[0] << " [options]" << std::endl;
  out << "Options:" << std::endl;
  for (const auto &option : option_map) {
    out << "  " << option.first << " " << std::get<2>(option.second)
        << std::endl;
  }
  exit(0);
}