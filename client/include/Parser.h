#pragma once

#include <string>
#include <vector>
#include <ctime>

#include "StompProtocol.h"
#include "Event.h"


class Parser
{
public:
    static bool quitApp();
    static void parseCommand(const std::string&, StompProtocol&);

private:
    static bool _sQuit;

    static std::vector<std::string> parseArgs(const std::string& input);
    static void writeSummary(const std::string& fileName, const std::vector<Event>& reports);
    static std::string epochToString(time_t val);

    static void login(const std::vector<std::string>&, StompProtocol&);
    static void join(const std::vector<std::string>&, StompProtocol&);
    static void exit(const std::vector<std::string>&, StompProtocol&);
    static void report(const std::vector<std::string>&, StompProtocol&);
    static void summary(const std::vector<std::string>&, StompProtocol&);
    static void logout(const std::vector<std::string>&, StompProtocol&);
    static void quit(const std::vector<std::string>&, StompProtocol&);
};