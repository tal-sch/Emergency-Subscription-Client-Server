#pragma once

#include <istream>
#include <string>
#include <vector>


class Utils
{
public:
    static std::vector<std::string> split_str(const std::string& str, const char delim);
    static std::vector<std::string> parseArgs(const std::string& input);
    static int generateReceiptID();
};