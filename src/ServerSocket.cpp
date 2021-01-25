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
#ifdef _WIN32
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	// TODO: add a logger and log specific error messages upon failure
	// WSAStartup returns zero if it was successful, return one of 5 error codes if it wasn't
	//           WSASYSNOTREADY    : Underlying network subsystem isn't ready for network communication
	//           WSAVERNOTSUPPORTED: The version of WinSock support requested in `ver` isn't supported by
	//                               this WinSock implementation
	//           WSAEINPROGRESS    : A blocking WinSock 1.1 operation is executing
	//           WSAEPRCLIM        : Maximum number of tasks supported by this WinSock implementation is reached
	//           WSAEFAULT         : The lpWSAData paramter (`&data`) isn't a valid pointer
	if (!WSAStartup(ver, &data))
		return false;

#endif

	return true;
}

bool ServerSocket::ServClose()
{
#ifdef _WIN32
	// WSACleanup returns zero if it was successful, return one of 3 error code if it wasn't.
	//           WSANOTINITIALISED : If cleanup was called before startup.
	//           WSAENETDOWN       : If network subsystem failed.
	//           WSAEINPROGRESS    : A blocking WinSock 1.1 call is in progress or service provider not done
	//                               processing callback function.
	return WSACleanup() != SOCKET_ERROR;
#endif
}

bool ServerSocket::ServSend(const uint32_t client_socket, const std::string& msg)
{
#ifdef _WIN32
	return send(client_socket, msg.c_str(), msg.size() + 1, 0) != SOCKET_ERROR;
#endif
}

#ifdef _WIN32
SOCKET ServerSocket::ServMakeSocket()
{
	addrinfo* addr_info = nullptr;
	addrinfo hints;

	// Set address hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Attempt to get address info (DNS lookup), close server and return invalid socket if it fails
	if (getaddrinfo(nullptr, std::to_string(this->port).c_str(), &hints, &addr_info) != 0)
	{
		this->ServClose();
		return INVALID_SOCKET;
	}

	// Make a listening socket given the address info returned by getaddrinfo
	// Close server and return invalid socket if listener creation fails
	SOCKET listener_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
	if (listener_sock == INVALID_SOCKET)
	{
		freeaddrinfo(addr_info);
		this->ServClose();
		return INVALID_SOCKET;
	}

	// Attempt to bind listener_sock to addr_info->ai_addr, which represents the server's IP address and port.
	// If binding is successful, bind returns zero.
	// If it isn't, bind returns SOCKET_ERROR.
	if (bind(listener_sock, addr_info->ai_addr, (int32_t)addr_info->ai_addrlen) == SOCKET_ERROR)
	{
		freeaddrinfo(addr_info);
		closesocket(listener_sock);
		this->ServClose();
		return INVALID_SOCKET;
	}

	// If binding is successful, addr_info is no longer needed and it's safe to free it
	freeaddrinfo(addr_info);
	return listener_sock;
}
#endif
