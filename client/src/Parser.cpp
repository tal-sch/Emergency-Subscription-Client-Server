#include "Parser.h"

#include <unordered_map>
#include <iostream>
#include <sstream>
#include <exception>

using Command = void (*)(const std::vector<std::string>&, StompProtocol&);

bool Parser::_sQuit = false;


bool Parser::quitApp()
{
    return _sQuit;
}

void Parser::parseCommand(const std::string &input, StompProtocol &protocol)
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
        {"logout", {Parser::logout, 1}},
        {"quit", {Parser::quit, 1}}
    };

    std::vector<std::string> args = parseArgs(input);
    if (args.empty()) return;

    auto it = commands.find(args.front());

    if (it == commands.end()) {
        std::cerr << "Invalid command: '" << args.front() << "'\n";
        return;
    } else if (args.size() < it->second.second) {
        std::cerr << "Command '" << it->first
                  << "' requires " << it->second.second - 1 << " argument(s)\n";
        return;
    }

    try {
        (*it->second.first)(args, protocol);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

std::vector<std::string> Parser::parseArgs(const std::string &input)
{
    std::istringstream stream(input);
    std::string arg;
    std::vector<std::string> args;
    while (stream >> arg) args.push_back(arg);
    return args;
}

void Parser::login(const std::vector<std::string> &args, StompProtocol &protocol)
{
    std::string address = args[1];
    std::string user = args[2];
    std::string password = args[3];

    size_t colonPos = address.find(':');

    if (colonPos == std::string::npos)
        throw std::invalid_argument("Invalid address: '" + address + '\'');
    
    protocol.login(
        address.substr(0, colonPos),
        static_cast<short>(std::stoi(address.substr(colonPos + 1))),
        user,
        password
    );
}

void Parser::join(const std::vector<std::string>& args, StompProtocol& protocol)
{
}

void Parser::exit(const std::vector<std::string>& args, StompProtocol& protocol)
{
}

void Parser::report(const std::vector<std::string>& args, StompProtocol& protocol)
{
}

void Parser::summary(const std::vector<std::string>& args, StompProtocol& protocol)
{
}

void Parser::logout(const std::vector<std::string>& args, StompProtocol& protocol)
{
    protocol.logout();
}

void Parser::quit(const std::vector<std::string> &, StompProtocol &)
{
    _sQuit = true;
}
