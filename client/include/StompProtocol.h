#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "ConnectionHandler.h"
#include "Event.h"


enum class FrameType
{
    CONNECT,
    SEND,
    SUBSCRIBE,
    UNSUBSCRIBE,
    DISCONNECT,
    CONNECTED,
    MESSAGE,
    RECEIPT,
    ERROR
};

class Frame
{
public:
    Frame(FrameType type, const std::unordered_map<std::string, std::string>& headers, const std::string& body);
    FrameType type() const;
    const std::string& getHeader(const std::string& header) const;
    const std::string& body() const;

    std::string toString() const;
    static Frame parseFrame(char bytes[]);
    
private:
    FrameType _type;
    std::unordered_map<std::string, std::string> _headers;
    std::string _body;
};

class StompProtocol
{
public:
    StompProtocol();
    bool isLoggedIn() const;

private:
    std::unique_ptr<ConnectionHandler> _pConnection;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> _data;
};
