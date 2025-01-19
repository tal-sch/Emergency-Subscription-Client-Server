#include "StompProtocol.h"

#include <unordered_map>
#include <sstream>
#include <exception>


Frame::Frame(FrameType type, const std::unordered_map<std::string, std::string> &headers, const std::string &body)
    : _type(type)
    , _headers(headers)
    , _body(body)
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

std::string Frame::toString() const
{
    std::string frame = getFrameName(_type) + '\n';
    
    for (auto header : _headers)
        frame.append(header.first + ':' + header.second);

    frame.append(1, '\n');
    frame.append(_body);
    
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
    : _pConnection()
    , _data()
{
}

void StompProtocol::closeConnection()
{
    _pConnection.release();
}

Frame StompProtocol::receiveFrame()
{
    std::string buffer;
    _pConnection->readFrame(buffer);
    return Frame::parseFrame(buffer);
}

void StompProtocol::login(const std::string &host, short port, const std::string &username, const std::string &password)
{
    _pConnection = std::unique_ptr<ConnectionHandler>(new ConnectionHandler(host, port));

    if (!_pConnection->connect())
        return;

    _pConnection->sendFrame(Frame::connectFrame(username, password).toString());
}

void StompProtocol::logout(int receipt)
{
    _pConnection->sendFrame(Frame::disconnectFrame(receipt).toString());
}

const char *Frame::getFrameName(FrameType t)
{
    static const char* names[] = {
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

Frame Frame::connectFrame(const std::string &user, const std::string &password)
{
    std::unordered_map<std::string, std::string> headers = {
        {"login", user},
        {"passcode", password},
        {"accept-version", "1.2"},
        {"host", "stomp.cs.bgu.ac.il"}
    };

    return Frame(FrameType::CONNECT, headers, std::string());
}

Frame Frame::disconnectFrame(int receipt)
{
    return Frame(
        FrameType::DISCONNECT,
        {{"receipt", std::to_string(receipt)}},
        std::string()
    );
}
