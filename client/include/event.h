#pragma once

#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>


class Event
{
public:
    Event(std::string channel_name, std::string city, std::string name, int date_time, std::string description, std::map<std::string, std::string> general_information);
    Event(const std::string & frame_body);
    void setEventOwnerUser(std::string setEventOwnerUser);
    const std::string &getEventOwnerUser() const;
    const std::string &get_channel_name() const;
    const std::string &get_city() const;
    const std::string &get_description() const;
    const std::string &get_name() const;
    int get_date_time() const;
    const std::map<std::string, std::string> &get_general_information() const;
    
private:
    std::string channel_name; // name of channel
    std::string city; // city of the event 
    std::string name; // name of the event
    int datetime; // time of the event in seconds
    std::string description; // description of the event
    std::map<std::string, std::string> general_info; // map of all the general information
    std::string event_owner;

    static std::unordered_map<std::string, std::string> parseFrameBody(const std::string& frameBody);
    static std::map<std::string, std::string> parseGeneralInfo(const std::string& info);
};

// an object that holds the names of the teams and a vector of events, to be returned by the parseEventsFile function
struct names_and_events {
    std::string channel_name;
    std::vector<Event> events;
};

// function that parses the json file and returns a names_and_events object
names_and_events parseEventsFile(std::string json_path);
