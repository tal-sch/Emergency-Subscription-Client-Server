#include "Utils.h"

#include <sstream>


std::vector<std::string> Utils::split_str(const std::string &str, const char delim)
{
    std::istringstream stream(str);
    std::string segment;
    std::vector<std::string> args;
    while (std::getline(stream, segment, delim)) args.push_back(segment);
    return args;
}

std::vector<std::string> Utils::parseArgs(const std::string &input)
{
    std::istringstream stream(input);
    std::string arg;
    std::vector<std::string> args;
    while (stream >> arg) args.push_back(arg);
    return args;
}
