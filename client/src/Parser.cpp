#include "Parser.h"

#include <unordered_map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <ctime>
#include <iomanip>
#include <set>

using Command = void (*)(const std::vector<std::string>&, StompProtocol&);

bool Parser::_sQuit = false;


bool Parser::shouldQuit()
{
    return _sQuit;
}

void Parser::parseCommand(const std::string &input, StompProtocol &protocol)
{
    if (input.empty())
        return;

    static const std::unordered_map<std::string, std::pair<Command, size_t>> commands = {
        // {command name, {command function, No. of required arguments}}
        {"login", {Parser::login, 4}},
        {"join", {Parser::join, 2}},
        {"exit", {Parser::exit, 2}},
        {"report", {Parser::report, 2}},
        {"summary", {Parser::summary, 4}},
        {"logout", {Parser::logout, 1}},
        {"quit", {Parser::quit, 1}}
    };

    std::vector<std::string> args = parseArgs(input);
    if (args.empty()) return;

    auto it = commands.find(args.front());

    if (it == commands.end()) {
        std::cerr << "Invalid command: '" << args.front() << "'\n";
        return;
    } else if (args.size() < it->second.second) {
        std::cerr << "Command '" << it->first
                  << "' requires " << it->second.second - 1 << " argument(s)\n";
        return;
    }

    try {
        (*it->second.first)(args, protocol);

    } catch (boost::system::system_error& e) {
        std::cerr << "Socket Error: " << e.what() << '\n';
        protocol.closeConnection();

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

std::vector<std::string> Parser::parseArgs(const std::string &input)
{
    std::istringstream stream(input);
    std::string arg;
    std::vector<std::string> args;
    while (stream >> arg) args.push_back(arg);
    return args;
}

void Parser::writeSummary(const std::string &fileName, const std::vector<Event> &reports)
{
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

std::string Parser::epochToString(time_t val)
{
    std::tm* time = std::localtime(&val);
    std::ostringstream stream;
    stream << std::put_time(time, "%Y-%m-%d %H:%M");
    return stream.str();
}

void Parser::login(const std::vector<std::string> &args, StompProtocol &protocol)
{
    std::string address = args[1];
    std::string user = args[2];
    std::string password = args[3];

    size_t colonPos = address.find(':');

    if (colonPos == std::string::npos)
        throw std::invalid_argument("Invalid address: '" + address + '\'');
    
    protocol.login(
        address.substr(0, colonPos),
        static_cast<short>(std::stoi(address.substr(colonPos + 1))),
        user,
        password
    );
}

void Parser::join(const std::vector<std::string>& args, StompProtocol& protocol)
{
    protocol.subscribe(args[1]);
}

void Parser::exit(const std::vector<std::string>& args, StompProtocol& protocol)
{
    protocol.unsubscribe(args[1]);
}

void Parser::report(const std::vector<std::string>& args, StompProtocol& protocol)
{
    const std::string& file = args[1];
    std::vector<Event> events = Event::fromJsonFile(file);

    if (events.empty())
        return;

    for (Event& event : events)
        protocol.report(event);

    std::cout << "Events reported\n";
}

void Parser::summary(const std::vector<std::string>& args, StompProtocol& protocol)
{
    const std::string& channel = args[1];
    const std::string& user = args[2];
    const std::string& file = args[3];

    std::vector<Event> reports = protocol.getReportsFrom(channel, user);

    if (reports.empty()) {
        std::cout << "Nothing to summarize\n";
        return;
    }

    writeSummary(file, reports);
}

void Parser::logout(const std::vector<std::string>& args, StompProtocol& protocol)
{
    protocol.logout();
}

void Parser::quit(const std::vector<std::string> &, StompProtocol &)
{
    _sQuit = true;
}
