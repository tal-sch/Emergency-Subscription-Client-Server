#include "ConnectionHandler.h"

#include <sstream>
#include <iostream>


ConnectionHandler::ConnectionHandler(std::string host, short port)
	: _host(host)
	, _port(port)
	, _ioService()
	, _socket(_ioService)
{
}

ConnectionHandler::~ConnectionHandler()
{
	close();
}

bool ConnectionHandler::connect()
{
	boost::asio::ip::tcp::endpoint endpoint(
		boost::asio::ip::address::from_string(_host),
		_port);

	boost::system::error_code ec;
	_socket.connect(endpoint, ec);

	if (ec) {
		std::cerr << "Error: server is not running\n";
		_socket.close();
		return false;
	}

	return true;
}

// Close down the connection properly.
void ConnectionHandler::close()
{
	boost::system::error_code error;
	_socket.close(error);
	if (error) std::cerr << "Error while closing socket: " << error.message() << '\n';
}

std::string ConnectionHandler::readFrame()
{
	boost::asio::streambuf buffer;
	boost::asio::read_until(_socket, buffer, '\0');
	return std::string(std::istreambuf_iterator<char>(&buffer), std::istreambuf_iterator<char>());
}

void ConnectionHandler::sendFrame(const std::string& data)
{
	_socket.send(boost::asio::buffer(data));
}

bool ConnectionHandler::isConnected() const
{
    return _socket.is_open();
}

void ConnectionHandler::waitForData()
{
	boost::system::error_code ec;
	_socket.wait(boost::asio::socket_base::wait_read, ec);
}

bool ConnectionHandler::isDataAvailable()
{
    return isConnected() && _socket.available() > 0;
}
