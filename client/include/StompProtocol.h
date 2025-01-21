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
    FrameType type() const;
    const std::string getHeader(const std::string& header) const;
    const std::string& body() const;

    std::string raw() const;
    static Frame parseFrame(const std::string& frame);
    static const std::string& getFrameName(FrameType t);
    static FrameType getFrameType(const std::string& name);

    static Frame Connect(const std::string& user, const std::string& password);
    static Frame Disconnect(int receipt);
    static Frame Subscribe(const std::string& topic, int id, int receipt);
    
private:
    FrameType _type;
    std::unordered_map<std::string, std::string> _headers;
    std::string _body;

    Frame(FrameType type, const std::unordered_map<std::string, std::string>& headers);
    Frame(FrameType type, const std::unordered_map<std::string, std::string>& headers, const std::string& body);
};

class StompProtocol
{
public:
    StompProtocol();

    void closeConnection();

    void login(const std::string& host, short port, const std::string& username, const std::string& password);
    void logout();
    void subscribe(const std::string& topic);

private:
    bool _loggedIn;
    std::string _username;
    std::unique_ptr<ConnectionHandler> _pConnection;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> _data;

    void send(const Frame& frame);
    Frame recv();

    static int generateReceiptID();
    int generateSubscriptionID(const std::string& topic);
};
