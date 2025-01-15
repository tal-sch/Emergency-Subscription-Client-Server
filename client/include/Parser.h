#pragma once

#include <string>
#include <vector>


class Parser
{
public:
    static void parseCommand(const std::string& input);

private:
    static void login(std::vector<std::string> args);
    static void join(std::vector<std::string> args);
    static void exit(std::vector<std::string> args);
    static void report(std::vector<std::string> args);
    static void summary(std::vector<std::string> args);
    static void logout(std::vector<std::string> args);
};