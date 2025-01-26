#include "StompProtocol.h"

#include <unordered_map>
#include <sstream>
#include <exception>
#include <random>
#include <thread>
#include <iostream>
#include <algorithm>


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
    : _ioContext()
    , _socket(_ioContext)
    , _mtxSocket()
    , _pLastFrame()
    , _loggedIn(false)
    , _username()
    , _subscriptions()
    , _data()
    , _mtxData()
{
}

StompProtocol::~StompProtocol()
{   
    closeConnection();
}

void StompProtocol::closeConnection()
{
    boost::system::error_code ec;
    _socket.close(ec);
    _loggedIn.store(false);
    _username.clear();
    _subscriptions.clear();
    _pLastFrame.reset();
}

void StompProtocol::closeConnectionLogout()
{
    logout();
    closeConnection();
}

std::vector<Event> StompProtocol::getReportsFrom(const std::string &channel, const std::string &user)
{
    std::lock_guard<std::mutex> lck(_mtxData);
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
    if (_loggedIn.load())
        throw std::logic_error("Already logged in");

    if (!_socket.is_open()) {
        boost::asio::ip::tcp::endpoint ep(
		    boost::asio::ip::address::from_string(host),
		    port
        );

        boost::system::error_code ec;
        _socket.connect(ep, ec);

        if (ec) {
            std::cerr << "Server is not running\n";
            _socket.close();
            return;
        }
    }

    send(Frame::Connect(username, password));

    std::thread reader(&StompProtocol::receiveMessages, this);
    reader.detach();
}

void StompProtocol::logout()
{
    if (!_loggedIn.load())
        throw std::logic_error("Not logged in");
    
    send(Frame::Disconnect(generateReceiptID()));
}

void StompProtocol::subscribe(const std::string &topic)
{
    if (!_loggedIn.load())
        throw std::logic_error("Not logged in");

    if (_subscriptions.find(topic) != _subscriptions.end())
        throw std::invalid_argument("Already subscribed to '" + topic + '\'');

    send(Frame::Subscribe(topic, generateSubscriptionID(topic), generateReceiptID()));
}

void StompProtocol::unsubscribe(const std::string &topic)
{
    if (!_loggedIn.load())
        throw std::logic_error("Not logged in");

    auto it = _subscriptions.find(topic);
    
    if (it == _subscriptions.end())
        throw std::invalid_argument("Not subscribed to '" + topic + '\'');

    send(Frame::Unsubscribe(it->second, generateReceiptID()));
    _subscriptions.erase(it);
    std::cout << "Exited '" << topic << "'\n";
}

void StompProtocol::report(Event &event)
{
    if (!_loggedIn.load())
        throw std::logic_error("Not logged in");

    if (_subscriptions.find(event.get_channel_name()) == _subscriptions.end())
        throw std::invalid_argument("Not subscribed to '" + event.get_channel_name() + '\'');

    event.setEventOwnerUser(_username);
    send(Frame::Send(event));
}

void StompProtocol::send(const Frame &frame)
{
    boost::system::error_code ec;
    _socket.send(boost::asio::buffer(frame.raw()), 0, ec);

    if (ec) {
        std::cerr << "Socket Error: " << ec.message() << '\n';
        closeConnection();
    }

    _pLastFrame.reset(new Frame(frame));
}

std::string StompProtocol::readFrame()
{
    std::string frame;
    char c;

    do {
        _socket.read_some(boost::asio::buffer(&c, 1));
        frame.append(1, c);
    } while (c != '\0');

    frame.pop_back();
    return frame;
}

void StompProtocol::receiveMessages()
{
    do {
        try {
            Frame f = Frame::parseFrame(readFrame());

            switch (f.type()) {
                case FrameType::CONNECTED:
                    handleConnected(f);
                    break;

                case FrameType::RECEIPT:
                    handleReceipt(f);
                    break;

                case FrameType::MESSAGE:
                    handleMessage(f);
                    break;

                case FrameType::ERROR:
                    std::cout << f.getHeader("message") << '\n';
                    break;

                default:
                    std::cerr << "Unhandled frame received: " << (int)f.type() << '\n';
                    break;
            }
        } catch (boost::system::system_error& e) {
            std::cerr << e.what() << '\n';
            closeConnection();
        } catch (std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    } while (_loggedIn.load());
}

void StompProtocol::handleConnected(Frame f)
{
    std::cout << "Login successful\n";
    _loggedIn.store(true);
    _username = _pLastFrame->getHeader("login");
}

void StompProtocol::handleReceipt(Frame f)
{
    if (f.getHeader("receipt-id") != _pLastFrame->getHeader("receipt"))
        return;

    std::string channel;

    switch (_pLastFrame->type()) {
        case FrameType::DISCONNECT:
            std::cout << "Logout successful\n";
            closeConnection();
            break;

        case FrameType::SUBSCRIBE:
            channel = _pLastFrame->getHeader("destination");
            std::cout << "Subscribed to '" << channel << "'\n";
            _subscriptions[channel] = std::stoi(_pLastFrame->getHeader("id"));
            break;

        case FrameType::UNSUBSCRIBE:
            // already handled
            break;

        default:
            std::cout << "Received receipt of unknown purpose\n";
            break;
    }
}

void StompProtocol::handleMessage(Frame f)
{
    Event e(f.body());
    std::string channelName = f.getHeader("destination").substr(1);
    e.setChannelName(channelName);
    std::lock_guard<std::mutex> lck(_mtxData);
    _data[channelName][e.getEventOwnerUser()].push_back(e);
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
        {{"receipt", std::to_string(receipt)}, {"destination", '/' + event.get_channel_name()}},
        event.toString()
    );
}

Frame Frame::Send(const Event &event)
{
    return Frame(
        FrameType::SEND,
        {{"destination", '/' + event.get_channel_name()}},
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
