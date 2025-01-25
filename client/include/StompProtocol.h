#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <mutex>
#include <boost/asio.hpp>

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
    static Frame Unsubscribe(int id, int receipt);
    static Frame Send(const Event& event, int receipt);
    
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
    ~StompProtocol();
    
    void closeConnection();
    void closeConnectionLogout();
    std::vector<Event> getReportsFrom(const std::string& channel, const std::string& user);

    void login(const std::string& host, short port, const std::string& username, const std::string& password);
    void logout();
    void subscribe(const std::string& topic);
    void unsubscribe(const std::string& topic);
    void report(Event& event);

private:
    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::socket _socket;
    std::mutex _mtxSocket;


    std::atomic<bool> _loggedIn;
    std::string _username;
    std::unordered_map<std::string, size_t> _subscriptions;
    
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> _data;
    std::mutex _mtxData;
    
    void send(const Frame& frame);
    Frame recv();
    Frame safeSendReceive(const Frame& frame);

    void reportCallback(const boost::system::error_code& ec);

    static int generateReceiptID();
    size_t generateSubscriptionID(const std::string& topic);

};
