#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ConnectionHandler.h"
#include "event.h"


class StompProtocol
{
public:
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
        Frame(FrameType type, std::unordered_map<std::string, std::string> headers, std::string body);

        FrameType type() const;
        const std::string& getHeader(const std::string& header) const;
        const std::string& body() const;

    private:
        FrameType type_;
        std::unordered_map<std::string, std::string> headers_;
        std::string body_;
    };

    StompProtocol(ConnectionHandler& connection);

private:
    ConnectionHandler& connection_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> data_;
};
