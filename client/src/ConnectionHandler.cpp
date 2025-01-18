#include "ConnectionHandler.h"

using boost::asio::ip::tcp;


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
	std::cout << "Connecting to "
	          << _host << ":" << _port << '\n';

	tcp::endpoint endpoint(
		boost::asio::ip::address::from_string(_host),
		_port); // the server endpoint

	boost::system::error_code error;
	_socket.connect(endpoint, error);
	
	if (error) {
		std::cerr << "Connection failed (Error: " << error.message() << ")\n";
		return false;
	}

	return true;
}

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead)
{
	size_t tmp = 0;
	boost::system::error_code error;

	while (!error && bytesToRead > tmp) {
		tmp += _socket.read_some(
			boost::asio::buffer(bytes + tmp, bytesToRead - tmp),
			error
		);
	}
	
	if (error) {
		std::cerr << "recv failed (Error: " << error.message() << ")\n";
		return false;
	}
	
	return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite)
{
	int tmp = 0;
	boost::system::error_code error;

	while (!error && bytesToWrite > tmp) {
		tmp += _socket.write_some(
			boost::asio::buffer(bytes + tmp, bytesToWrite - tmp),
			error
		);
	}

	if (error) {
		std::cerr << "recv failed (Error: " << error.message() << ")\n";
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
	// Stop when we encounter the null character.
	// Notice that the null character is not appended to the frame string.
	try {
		do {
			if (!getBytes(&ch, 1)) return false;
			if (ch != '\0') frame.append(1, ch);
		} while (delimiter != ch);
	} catch (std::exception &e) {
		std::cerr << "recv failed2 (Error: " << e.what() << ")\n";
		return false;
	}

	return true;
}

bool ConnectionHandler::sendFrameAscii(const std::string &frame, char delimiter)
{
	bool result = sendBytes(frame.c_str(), frame.length());
	if (!result) return false;
	return sendBytes(&delimiter, 1);
}

// Close down the connection properly.
void ConnectionHandler::close()
{
	try {
		_socket.close();
	} catch (...) {
		std::cout << "closing failed: connection already closed" << std::endl;
	}
}
