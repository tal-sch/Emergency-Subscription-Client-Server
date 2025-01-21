#include "StompProtocol.h"

#include <unordered_map>
#include <sstream>
#include <exception>
#include <random>


Frame::Frame(FrameType type, const std::unordered_map<std::string, std::string> &headers)
    : Frame(type, headers, std::string())
{
}

Frame::Frame(FrameType type, const std::unordered_map<std::string, std::string> &headers, const std::string &body)
    : _type(type), _headers(headers), _body(body)
{
}

FrameType Frame::type() const
{
    return _type;
}

const std::string Frame::getHeader(const std::string &header) const
{
    auto it = _headers.find(header);
    return (it != _headers.end()) ? it->second : std::string();
}

const std::string &Frame::body() const
{
    return _body;
}

std::string Frame::raw() const
{
    std::string frame = getFrameName(_type) + '\n';
    
    for (auto header : _headers)
        frame.append(header.first + ':' + header.second + '\n');

    frame.append(1, '\n');
    frame.append(_body);
    frame.append(1, '\0');
    
    return frame;
}

Frame Frame::parseFrame(const std::string &frame)
{
    std::istringstream stream(frame);
    std::string line;

    std::getline(stream, line);

    FrameType type = getFrameType(line);

    std::unordered_map<std::string, std::string> headers;

    while (stream.peek() != '\n') {
        std::getline(stream, line);
        size_t colonPos = line.find(':');
        headers[line.substr(0, colonPos)] = line.substr(colonPos + 1);
    }

    std::getline(stream, line); // empty line

    std::string body;

    while (std::getline(stream, line))
        body.append(line + '\n');

    return Frame(type, headers, body);
}

StompProtocol::StompProtocol()
    : _loggedIn(false)
    , _username()
    , _pConnection()
    , _data()
    , _subscriptions()
{
}

void StompProtocol::closeConnection()
{
    _pConnection.release();
    _username.clear();
    _subscriptions.clear();
    _loggedIn = false;
}

std::vector<Event> StompProtocol::getReportsFrom(const std::string &channel, const std::string &user)
{
    auto channelIt = _data.find(channel);

    if (channelIt == _data.end())
        return std::vector<Event>();

    auto channelData = channelIt->second;
    auto userIt = channelData.find(user);

    if (userIt == channelData.end())
        return std::vector<Event>();

    return userIt->second;
}

void StompProtocol::login(const std::string &host, short port, const std::string &username, const std::string &password)
{
    if (_loggedIn)
        throw std::logic_error("Already logged in");

    if (_pConnection == nullptr)
        _pConnection = std::unique_ptr<ConnectionHandler>(new ConnectionHandler(host, port));
    
    if (!_pConnection->isConnected()) {
        if (!_pConnection->connect())
            return;
    }
    
    send(Frame::Connect(username, password));

    Frame response = recv();

    if (response.type() == FrameType::CONNECTED) {
        std::cout << "Login successful\n";
        _loggedIn = true;
        _username = username;
    } else {
        std::cout << response.getHeader("message") << '\n';
    }
}

void StompProtocol::logout()
{
    if (!_loggedIn)
        throw std::logic_error("Not logged in");
    
    int receipt = generateReceiptID();
    
    send(Frame::Disconnect(receipt));

    Frame response = recv();

    if (response.type() == FrameType::RECEIPT
        && response.getHeader("receipt-id") == std::to_string(receipt)) {
        std::cout << "Logout successful\n";
        closeConnection();
    }
}

void StompProtocol::subscribe(const std::string &topic)
{
    if (!_loggedIn)
        throw std::logic_error("Not logged in");

    if (_subscriptions.find(topic) != _subscriptions.end())
        throw std::invalid_argument("Already subscribed to '" + topic + '\'');

    size_t subID = generateSubscriptionID(topic);
    int receipt = generateReceiptID();

    send(Frame::Subscribe(topic, subID, receipt));

    Frame response = recv();

    if (response.type() == FrameType::RECEIPT
        && response.getHeader("receipt-id") == std::to_string(receipt)) {
        std::cout << "Joined channel '" << topic << "'\n";
        _subscriptions[topic] = subID;
    } else {
        std::cout << "Could not join channel '" << topic << "'\n"
                  << response.getHeader("message") << '\n';
    }
}

void StompProtocol::unsubscribe(const std::string &topic)
{
    if (!_loggedIn)
        throw std::logic_error("Not logged in");

    auto it = _subscriptions.find(topic);
    
    if (it == _subscriptions.end())
        throw std::invalid_argument("Not subscribed to '" + topic + '\'');

    int receipt = generateReceiptID();

    send(Frame::Unsubscribe(it->second, receipt));

    Frame response = recv();

    if (response.type() == FrameType::RECEIPT
        && response.getHeader("receipt-id") == std::to_string(receipt)) {
        std::cout << "Exited channel '" << topic << "'\n";
        _subscriptions.erase(it);
    } else {
        std::cout << "Could not exit channel '" << topic << "'\n"
                  << response.getHeader("message") << '\n';
    }
}

void StompProtocol::report(Event &event)
{
    if (!_loggedIn)
        throw std::logic_error("Not logged in");

    int receipt = generateReceiptID();

    event.setEventOwnerUser(_username);

    std::cout << "reporting " << event.get_name() << '\n';
    send(Frame::Send(event,receipt));

    Frame response = recv();

    if (response.type() == FrameType::RECEIPT
        && response.getHeader("receipt-id") == std::to_string(receipt)) {
        _data[event.get_channel_name()][event.getEventOwnerUser()].push_back(event);
    } else {
        std::cout << "Error: reporting event failed\n"
                  << response.body() << '\n';
    }
    
}

void StompProtocol::send(const Frame &frame)
{
    _pConnection->sendFrame(frame.raw());
}

Frame StompProtocol::recv()
{
    return Frame::parseFrame(_pConnection->readFrame());
}

const std::string& Frame::getFrameName(FrameType t)
{
    static const std::string names[] = {
        "CONNECT",
        "SEND",
        "SUBSCRIBE",
        "UNSUBSCRIBE",
        "DISCONNECT",
        "CONNECTED",
        "MESSAGE",
        "RECEIPT",
        "ERROR"
    };

    return names[static_cast<size_t>(t)];
}

FrameType Frame::getFrameType(const std::string &name)
{
    static const std::unordered_map<std::string, FrameType> types = {
        {"CONNECT", FrameType::CONNECT},
        {"SEND", FrameType::SEND},
        {"SUBSCRIBE", FrameType::SUBSCRIBE},
        {"UNSUBSCRIBE", FrameType::UNSUBSCRIBE},
        {"DISCONNECT", FrameType::DISCONNECT},
        {"CONNECTED", FrameType::CONNECTED},
        {"MESSAGE", FrameType::MESSAGE},
        {"RECEIPT", FrameType::RECEIPT},
        {"ERROR", FrameType::ERROR}
    };

    auto it = types.find(name);

    if (it == types.end())
        throw std::invalid_argument('\'' + name + "' is not a frame type");

    return it->second;
}

Frame Frame::Connect(const std::string &user, const std::string &password)
{
    std::unordered_map<std::string, std::string> headers = {
        {"login", user},
        {"passcode", password},
        {"accept-version", "1.2"},
        {"host", "stomp.cs.bgu.ac.il"}
    };

    return Frame(FrameType::CONNECT, headers);
}

Frame Frame::Disconnect(int receipt)
{
    return Frame(
        FrameType::DISCONNECT,
        {{"receipt", std::to_string(receipt)}}
    );
}

Frame Frame::Subscribe(const std::string &topic, int id, int receipt)
{
    std::unordered_map<std::string, std::string> headers = {
        {"destination", topic},
        {"id", std::to_string(id)},
        {"receipt", std::to_string(receipt)}
    };

    return Frame(FrameType::SUBSCRIBE, headers);
}

Frame Frame::Unsubscribe(int id, int receipt)
{
    return Frame(
        FrameType::UNSUBSCRIBE,
        {{"id", std::to_string(id)}, {"receipt", std::to_string(receipt)}}
    );
}

Frame Frame::Send(const Event &event, int receipt)
{
    return Frame(
        FrameType::SEND,
        {{"receipt", std::to_string(receipt)}, {"destination", "/" + event.get_channel_name()}},
        event.toString()
    );
}

int StompProtocol::generateReceiptID()
{
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(10, 99);
    return distribution(generator);
}

size_t StompProtocol::generateSubscriptionID(const std::string &topic)
{
    std::string s = _username + topic;
    std::hash<std::string> hash;
    return hash(s);
}
