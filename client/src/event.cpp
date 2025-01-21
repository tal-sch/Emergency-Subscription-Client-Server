#include "Event.h"

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

#include "json.hpp"

using json = nlohmann::json;


Event::Event(std::string channel_name,
             std::string city,
             std::string name,
             int date_time,
             std::string description,
             std::map<std::string,
             std::string> general_information)
    : _channelName(channel_name)
    , _city(city)
    , _name(name)
    , _datetime(date_time)
    , _description(description)
    , _generalInfo(general_information)
    , _eventOwner()
{
}

void Event::setEventOwnerUser(std::string setEventOwnerUser)
{
    _eventOwner = setEventOwnerUser;
}

const std::string &Event::getEventOwnerUser() const
{
    return _eventOwner;
}

const std::string &Event::get_channel_name() const
{
    return _channelName;
}

const std::string &Event::get_city() const
{
    return _city;
}

const std::string &Event::get_name() const
{
    return _name;
}

int Event::get_date_time() const
{
    return _datetime;
}

const std::map<std::string, std::string> &Event::get_general_information() const
{
    return _generalInfo;
}

std::string Event::summary() const
{
    std::string summary = _description.substr(0, 27);
    if (summary.length() < _description.length()) summary.append("...");
    return summary;
}

std::string Event::toString() const
{
    std::ostringstream stream;

    stream << "user: " << _eventOwner << '\n'
           << "city: " << _city << '\n'
           << "event name: " << _name << '\n'
           << "date time: " << _datetime << '\n'
           << "general information:\n"
                << "\tactive: " << _generalInfo.at("active") << '\n'
                << "\tforces_arrival_at_scene: " << _generalInfo.at("forces_arrival_at_scene") << '\n'
           << "description: " << _description;

    return stream.str();
}

std::unordered_map<std::string, std::string> Event::parseFrameBody(const std::string &frameBody)
{
    std::istringstream stream(frameBody);
    std::string line;
    std::unordered_map<std::string, std::string> data;


    while (std::getline(stream, line)) {
        size_t colonPos = line.find(':');

        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);

            if (key == "general information") {
                std::string generalInfo;
                
                while (isspace(stream.peek())) {
                    std::getline(stream, line);
                    generalInfo.append(line.substr(1) + '\n');
                }

                data[key] = generalInfo;
                continue;
            }

            if (key == "description") {
                std::string description;
                while (std::getline(stream, line)) description.append(line + '\n');
                data[key] = description;
                break;
            }

            data[key] = line.substr(colonPos + 2);
        }
    }

    return data;
}

std::map<std::string, std::string> Event::parseGeneralInfo(const std::string &info)
{
    std::istringstream stream(info);
    std::string line;
    std::map<std::string, std::string> data;

    while (std::getline(stream, line)) {
        size_t colonPos = line.find(':');

        if (colonPos != std::string::npos)
            data[line.substr(0, colonPos)] = line.substr(colonPos + 2);
    }

    return data;
}

const std::string &Event::get_description() const
{
    return _description;
}

Event::Event(const std::string &frame_body)
    : _channelName()
    , _city()
    , _name()
    , _datetime(0)
    , _description()
    , _generalInfo()
    , _eventOwner()                        
                                            
{
    std::unordered_map<std::string, std::string> data = parseFrameBody(frame_body);
    std::unordered_map<std::string, std::string>::iterator it;

    if ((it = data.find("user")) != data.end()) _eventOwner = it->second;
    if ((it = data.find("channel name")) != data.end()) _channelName = it->second;
    if ((it = data.find("city")) != data.end()) _city = it->second;
    if ((it = data.find("event name")) != data.end()) _name = it->second;
    if ((it = data.find("date time")) != data.end()) _datetime = std::stoi(it->second);
    if ((it = data.find("general information")) != data.end()) _generalInfo = parseGeneralInfo(it->second);
    if ((it = data.find("description")) != data.end()) _description = it->second;
}

names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string channel_name = data["channel_name"];
    std::vector<Event> events;
    
    for (auto &event : data["events"]) {
        std::string name = event["event_name"];
        std::string city = event["city"];
        int date_time = event["date_time"];
        std::string description = event["description"];
        std::map<std::string, std::string> general_information;

        for (auto &update : event["general_information"].items()) {
            if (update.value().is_string())
                general_information[update.key()] = update.value();
            else
                general_information[update.key()] = update.value().dump();
        }

        events.emplace_back(
            channel_name,
            city,
            name,
            date_time,
            description,
            general_information
        );
    }

    return {channel_name, events};
}