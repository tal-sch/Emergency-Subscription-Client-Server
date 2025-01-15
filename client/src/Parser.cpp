#include "Parser.h"

#include <unordered_map>
#include <iostream>
#include <exception>

#include "Utils.h"

using Command = void (*)(std::vector<std::string>);


void Parser::parseCommand(const std::string &input)
{
    if (input.empty())
        return;

    static const std::unordered_map<std::string, std::pair<Command, size_t>> commands = {
        // {command name, {command function, No. of required arguments}}
        {"login", {Parser::login, 4}},
        {"join", {Parser::join, 2}},
        {"exit", {Parser::exit, 2}},
        {"report", {Parser::report, 2}},
        {"summary", {Parser::summary, 4}},
        {"logout", {Parser::logout, 1}}
    };

    std::vector<std::string> args = Utils::parseArgs(input);
    if (args.empty()) return;

    auto it = commands.find(args.front());

    if (it == commands.end()) {
        std::cerr << "Invalid command: '" << args.front() << '\n';
        return;
    } else if (args.size() < it->second.second) {
        std::cerr << "Command '" << it->first
                  << "' requires " << it->second.second - 1 << "arguments\n";
        return;
    }

    try {
        (*it->second.first)(args);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}