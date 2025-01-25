#include <iostream>
#include <vector>

#include "Event.h"


int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Missing json file path\n";
        return 1;
    }

    std::vector<Event> events = Event::fromJsonFile(argv[1]);

    for (Event& event : events) {
        event.setEventOwnerUser("curr_user");
        std::cout << event.toString() << '\n';
    }
    
    return 0;
}