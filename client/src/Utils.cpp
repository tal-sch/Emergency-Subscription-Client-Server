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