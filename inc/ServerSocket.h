#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	typedef SOCKET sock_t;
#endif

#ifdef __linux__
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	typedef uint32_t sock_t;
	#define INVALID_SOCKET  (sock_t)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <cstdint>
#include <string>
#include <functional>
#include <thread>

#define MAX_BUFFER_SIZE 15360

class ServerSocket
{
public:
	ServerSocket(const uint16_t, const uint32_t, std::function<void(ServerSocket*, const uint16_t, const std::string)>);
	ServerSocket(const std::string, const uint16_t, const uint32_t, std::function<void(ServerSocket*, const uint16_t, const std::string)>);
	ServerSocket() = delete;
	ServerSocket(const ServerSocket&) = delete;
	ServerSocket(ServerSocket&&) = delete;
	ServerSocket& operator=(const ServerSocket&) = delete;
	ServerSocket& operator=(ServerSocket&&) = delete;
	~ServerSocket();

	bool ServInit();
	bool ServRun();
	bool ServSend(const uint32_t, const std::string&);
	bool ServClose(const sock_t);

	// TODO: add setReceivedBufferSize(uint32_t);
private:
	std::string ipv4addr;
	uint16_t port;
	uint32_t maxNumOfConnections;
	std::function<void(ServerSocket*, const uint16_t, const std::string)> msgCallbackFunction;
	sock_t listenerSocket;
	sock_t ServMakeSocket();
	sock_t ServAccept(const sock_t);
};

#endif 