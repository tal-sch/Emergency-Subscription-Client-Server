#include "ConnectionHandler.h"

#include <sstream>


ConnectionHandler::ConnectionHandler(std::string host, short port)
	: _host(host)
	, _port(port)
	, _ioService()
	, _socket(_ioService)
	, _mtxSocket()
{
}

ConnectionHandler::~ConnectionHandler()
{
	close();
}

void ConnectionHandler::connect()
{
	std::cout << "Connecting to "
	          << _host << ":" << _port << '\n';

	boost::asio::ip::tcp::endpoint endpoint(
		boost::asio::ip::address::from_string(_host),
		_port);

	_socket.connect(endpoint);
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
