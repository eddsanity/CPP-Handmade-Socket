#include "ServerSocket.h"

ServerSocket::ServerSocket(const uint16_t p_port, std::function<void(ServerSocket, uint16_t, const std::string&)> p_callbackFunc)
{
	this->ipv4addr = "127.0.0.1";
	this->port = p_port;
	this->msgCallbackFunction = p_callbackFunc;
}

ServerSocket::~ServerSocket()
{
	this->ServClose();
}

bool ServerSocket::ServInit()
{
	if (this->msgCallbackFunction == nullptr)
		return false;

	// avoid system ports
	if (this->port <= 1023)
		return false;
	bool init_status = false;
#ifdef _WIN32
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
#endif
}

bool ServerSocket::ServSend(const uint32_t client_socket, const std::string& msg)
{
#ifdef _WIN32
	return (send(client_socket, msg.c_str(), msg.size() + 1, 0) != SOCKET_ERROR);
#endif
}

