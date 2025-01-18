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

private:
    FrameType _type;
    std::unordered_map<std::string, std::string> _headers;
    std::string _body;

    static Frame parseFrame(char bytes[]);
};

class StompProtocol
{
public:
    StompProtocol();
    bool isLoggedIn() const;
    bool exit() const;

private:
    std::unique_ptr<ConnectionHandler> _pConnection;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> _data;
    bool _exitApp;
};
