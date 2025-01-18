#pragma once

#include <string>
#include <vector>

#include "StompProtocol.h"


class Parser
{
public:
    static void parseCommand(const std::string&, StompProtocol&);

private:
    static void login(const std::vector<std::string>&, StompProtocol&);
    static void join(const std::vector<std::string>&, StompProtocol&);
    static void exit(const std::vector<std::string>&, StompProtocol&);
    static void report(const std::vector<std::string>&, StompProtocol&);
    static void summary(const std::vector<std::string>&, StompProtocol&);
    static void logout(const std::vector<std::string>&, StompProtocol&);
};