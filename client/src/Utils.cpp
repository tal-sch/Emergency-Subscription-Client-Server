#include "Utils.h"

#include <sstream>
#include <random>

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

int Utils::generateReceiptID()
{
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(10, 99);
    return distribution(generator);
}
