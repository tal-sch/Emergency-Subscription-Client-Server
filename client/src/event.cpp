#include "event.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstring>

#include "json.hpp"
#include "Utils.h"

using json = nlohmann::json;


Event::Event(std::string channel_name,
             std::string city,
             std::string name,
             int date_time,
             std::string description,
             std::map<std::string,
             std::string> general_information)
    : channelName_(channel_name)
    , city_(city)
    , name_(name)
    , datetime_(date_time)
    , description_(description)
    , generalInfo_(general_information)
    , eventOwner_("")
{
}

void Event::setEventOwnerUser(std::string setEventOwnerUser)
{
    eventOwner_ = setEventOwnerUser;
}

const std::string &Event::getEventOwnerUser() const
{
    return eventOwner_;
}

const std::string &Event::get_channel_name() const
{
    return channelName_;
}

const std::string &Event::get_city() const
{
    return city_;
}

const std::string &Event::get_name() const
{
    return name_;
}

int Event::get_date_time() const
{
    return datetime_;
}

const std::map<std::string, std::string> &Event::get_general_information() const
{
    return generalInfo_;
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
    return description_;
}

Event::Event(const std::string &frame_body)
    : channelName_()
    , city_()
    , name_()
    , datetime_(0)
    , description_()
    , generalInfo_()
    , eventOwner_()                        
                                            
{
    std::unordered_map<std::string, std::string> data = parseFrameBody(frame_body);
    std::unordered_map<std::string, std::string>::iterator it;

    if ((it = data.find("user")) != data.end()) eventOwner_ = it->second;
    if ((it = data.find("channel name")) != data.end()) channelName_ = it->second;
    if ((it = data.find("city")) != data.end()) city_ = it->second;
    if ((it = data.find("event name")) != data.end()) name_ = it->second;
    if ((it = data.find("date time")) != data.end()) datetime_ = std::stoi(it->second);
    if ((it = data.find("general information")) != data.end()) generalInfo_ = parseGeneralInfo(it->second);
    if ((it = data.find("description")) != data.end()) description_ = it->second;
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