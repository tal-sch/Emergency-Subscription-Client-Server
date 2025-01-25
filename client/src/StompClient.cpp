#include <iostream>
#include <string>

#include "Parser.h"
#include "StompProtocol.h"


int main()
{
    StompProtocol p;

    while (!Parser::shouldQuit()) {
        std::string line;
        std::getline(std::cin, line);
        Parser::parseCommand(line, p);
    }

    return 0;
}
