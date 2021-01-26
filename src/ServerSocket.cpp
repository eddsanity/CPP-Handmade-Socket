#include "ServerSocket.h"

ServerSocket::ServerSocket(const uint16_t p_port, const uint32_t backlog, std::function<void(ServerSocket*, const uint16_t, const std::string)> p_callbackFunc)
{
	this->ipv4addr = "127.0.0.1";
	this->port = p_port;
	this->maxNumOfConnections = (backlog == 0) ? SOMAXCONN : backlog;
	this->msgCallbackFunction = p_callbackFunc;
}

ServerSocket::ServerSocket(const std::string p_addr, const uint16_t p_port, const uint32_t backlog, std::function<void(ServerSocket*, const uint16_t, const std::string)> p_callbackFunc)
{
	this->ipv4addr = p_addr;
	this->port = p_port;
	this->maxNumOfConnections = (backlog == 0) ? SOMAXCONN : backlog;
	this->msgCallbackFunction = p_callbackFunc;
}

ServerSocket::~ServerSocket()
{
	this->ServClose(this->listenerSocket);
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
	if (WSAStartup(ver, &data) != 0)
		return false;

#endif

	return true;
}

bool ServerSocket::ServRun()
{
	// TODO: Multithread this
	sock_t listener_sock = INVALID_SOCKET;
	sock_t client_sock = INVALID_SOCKET;

	// buffer with maximum size of 15 KB
	char buf[MAX_BUFFER_SIZE];
	while (true)
	{
		// acquire listener
		listener_sock = this->ServMakeSocket();
		if (listener_sock == INVALID_SOCKET)
			return false;

		// accept client, if any
		client_sock = this->ServAccept(listener_sock);
		if (client_sock == INVALID_SOCKET)
			continue;

		closesocket(listener_sock);

		// recv returns 0 if nothing was received, the number of bytes received otherwise.
		// keep receiving messages until they stop coming and call the callback function on every message
		uint16_t bytes_received_cnt = 0;
		do
		{
			memset(buf, 0, MAX_BUFFER_SIZE);
			bytes_received_cnt = recv(client_sock, buf, MAX_BUFFER_SIZE, 0);
			if (bytes_received_cnt == 0)
				continue;

			// pass client message to callback function
			if (this->msgCallbackFunction != nullptr)
				this->msgCallbackFunction(this, client_sock, std::string(buf, 0, bytes_received_cnt));
		} while (bytes_received_cnt > 0);

		closesocket(client_sock);
	}
	return true;
}

bool ServerSocket::ServClose(const sock_t socket)
{
#ifdef _WIN32
	// WSACleanup returns zero if it was successful, return one of 3 error code if it wasn't.
	//           WSANOTINITIALISED : If cleanup was called before startup.
	//           WSAENETDOWN       : If network subsystem failed.
	//           WSAEINPROGRESS    : A blocking WinSock 1.1 call is in progress or service provider not done
	//                               processing callback function.
	return (closesocket(socket) == 0 && WSACleanup() != SOCKET_ERROR);
#endif

#ifdef __linux__
	return close(socket) == 0;
#endif
}

bool ServerSocket::ServSend(const uint32_t client_socket, const std::string& msg)
{
	return send(client_socket, msg.c_str(), msg.size() + 1, 0) != SOCKET_ERROR;
}

sock_t ServerSocket::ServMakeSocket()
{
	// Make an IPv4 TCP/IP listening sock_t with an unspecified protocol
	// Close server and return invalid sock_t if listener creation fails
	sock_t listener_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (listener_sock == INVALID_SOCKET)
	{
		this->ServClose(listener_sock);
		return INVALID_SOCKET;
	}

	sockaddr_in hints;
	// Set address hints
	ZeroMemory(&hints, sizeof(hints));
	hints.sin_family = AF_INET;
	hints.sin_port = htons(this->port);
	if (inet_pton(AF_INET, this->ipv4addr.c_str(), &hints.sin_addr) == 0)
		return INVALID_SOCKET;

	// Attempt to bind listener_sock to hints.sin_addr:hints.sin_port, which represents the server's IP address and port.
	// If binding is successful, bind returns zero.
	// If it isn't, bind returns SOCKET_ERROR.
	if (bind(listener_sock, (sockaddr*)&hints, sizeof(hints)) == SOCKET_ERROR)
	{
		this->ServClose(listener_sock);
		return INVALID_SOCKET;
	}

	// Start listening and return the socket
	if (listen(listener_sock, this->maxNumOfConnections) == SOCKET_ERROR)
	{
		this->ServClose(listener_sock);
		return INVALID_SOCKET;
	}
	return listener_sock;
}


sock_t ServerSocket::ServAccept(const sock_t p_listener_sock)
{
	return accept(p_listener_sock, nullptr, nullptr);
}

