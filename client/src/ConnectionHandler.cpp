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

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead)
{
	size_t bytesRead = 0;
	boost::system::error_code error;

	while (!error && bytesToRead > bytesRead) {
		bytesRead += _socket.read_some(
			boost::asio::buffer(bytes + bytesRead, bytesToRead - bytesRead),
			error
		);
	}
	
	if (error) {
		std::cerr << "recv failed (Error: " << error.message() << ")\n";
		return false;
	}
	
	return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], unsigned int bytesToWrite)
{
	size_t bytesWritten = 0;
	boost::system::error_code error;

	while (!error && bytesToWrite > bytesWritten) {
		bytesWritten += _socket.write_some(
			boost::asio::buffer(bytes + bytesWritten, bytesToWrite - bytesWritten),
			error
		);
	}

	if (error) {
		std::cerr << "send failed (Error: " << error.message() << ")\n";
		return false;
	}

	return true;
}

bool ConnectionHandler::getLine(std::string &line)
{
	return getFrameAscii(line, '\n');
}

bool ConnectionHandler::sendLine(std::string &line)
{
	return sendFrameAscii(line, '\n');
}


bool ConnectionHandler::getFrameAscii(std::string &frame, char delimiter)
{
	char ch;

	do {
		if (!getBytes(&ch, 1)) return false;
		if (ch != '\0') frame.append(1, ch);
	} while (ch != '\0' && ch != delimiter);

	return true;
}

bool ConnectionHandler::sendFrameAscii(const std::string &frame, char delimiter)
{
	bool result = sendBytes(frame.c_str(), frame.length());
	if (!result) return false;
	return sendBytes(&delimiter, 1);
}

bool ConnectionHandler::getByte(char *pData)
{
	boost::system::error_code ec;
	_socket.read_some(boost::asio::buffer(pData, 1), ec);

	if (ec) {
		std::cerr << "Error while reading from socket: " << ec.message() << '\n';
		return false;
	}

    return false;
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
