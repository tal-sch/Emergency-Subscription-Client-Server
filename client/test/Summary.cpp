#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "Event.h"


std::string epochToString(time_t val)
{
    std::tm* time = std::localtime(&val);
    std::ostringstream stream;
    stream << std::put_time(time, "%Y-%m-%d %H:%M");
    return stream.str();
}

void writeSummary(const std::string& fileName, const std::vector<Event>& reports)
{
    if (reports.empty()) return;
    std::string channelName = reports.front().get_channel_name();
    size_t activeCount = 0;
    size_t forcesArrivalCount = 0;

    static const auto cmp = [](const Event& a, const Event& b) {
        return a.get_date_time() < b.get_date_time();
    };

    std::set<Event, decltype(cmp)> sortedReports(cmp);

    for (const Event& report : reports) {
        const std::map<std::string, std::string>& info = report.get_general_information();
        bool active = (info.at("active") == "true") ? true : false;
        bool forcesArrival = (info.at("forces_arrival_at_scene") == "true") ? true : false;
        if (active) ++activeCount;
        if (forcesArrival) ++forcesArrivalCount;
        sortedReports.insert(report);
    }

    std::ofstream f(fileName);

    f << "Channel " << channelName << '\n'
      << "Stats:\nTotal: " << reports.size() << '\n'
      << "Active: " << activeCount << '\n'
      << "Forces arrival at scene: " << forcesArrivalCount << "\n\n";

    f << "Event Reports:\n\n";

    int counter = 1;

    for (const Event& report : sortedReports) {
        f << "Report_" << counter << ":\n\t"
          << "city: " << report.get_city() << "\n\t"
          << "date time: " << epochToString(report.get_date_time()) << "\n\t"
          << "event name: " << report.get_name() << "\n\t"
          << "summary: " << report.summary() << '\n';

        f << '\n';
        ++counter;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Missing output file path\n";
        return 1;
    }

    std::vector<Event> events = Event::fromJsonFile("data/events1.json");
    writeSummary(argv[1], events);

    return 0;
}