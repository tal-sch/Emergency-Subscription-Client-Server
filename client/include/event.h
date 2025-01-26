#pragma once

#include <string>
#include <ostream>
#include <map>
#include <unordered_map>
#include <vector>


class Event
{
public:
    Event(std::string channel_name, std::string city, std::string name, int date_time, std::string description, std::map<std::string, std::string> general_information);
    Event(const std::string & frame_body);
    void setEventOwnerUser(std::string setEventOwnerUser);
    void setChannelName(const std::string& channelName);
    const std::string &getEventOwnerUser() const;
    const std::string &get_channel_name() const;
    const std::string &get_city() const;
    const std::string &get_description() const;
    const std::string &get_name() const;
    int get_date_time() const;
    const std::map<std::string, std::string> &get_general_information() const;

    std::string summary() const;
    std::string toString() const;

    static std::vector<Event> fromJsonFile(const std::string& path);
    
private:
    std::string _channelName; // name of channel
    std::string _city; // city of the event 
    std::string _name; // name of the event
    int _datetime; // time of the event in seconds
    std::string _description; // description of the event
    std::map<std::string, std::string> _generalInfo; // map of all the general information
    std::string _eventOwner;

    static std::unordered_map<std::string, std::string> parseFrameBody(const std::string& frameBody);
    static std::map<std::string, std::string> parseGeneralInfo(const std::string& info);
};
