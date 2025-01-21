#pragma once

#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <mutex>


class ConnectionHandler
{
public:
	ConnectionHandler(std::string host, short port);
	~ConnectionHandler();

	bool connect();

	void close();

	std::string readFrame();
	void sendFrame(const std::string& data);

	bool isConnected() const;

private:
	const std::string _host;
	const short _port;
	boost::asio::io_service _ioService;
	boost::asio::ip::tcp::socket _socket;
	std::mutex _mtxSocket;
};
