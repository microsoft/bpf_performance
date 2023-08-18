// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class options
{
  public:
    typedef std::function<void(std::vector<std::string>::iterator)> option_handler;
    options(std::ostream& out = std::cout) : out(out)
    {
        add(
            "-h", 0, [this](auto iter) { this->print_help(); }, "Print this help message");
    }
    ~options() = default;

    void
    parse(int argc, char** argv);
    void
    add(const std::string& option, size_t num_args, option_handler func, const std::string& help);
    void
    print_help();

  private:
    std::map<std::string, std::tuple<size_t, option_handler, std::string>> option_map;
    std::vector<std::string> args;
    std::ostream& out;
};