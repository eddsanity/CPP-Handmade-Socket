#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
#endif

#include <cstdint>
#include <string>
#include <functional>
#include <thread>

#define MAX_BUFFER_SIZE 15360

class ServerSocket
{
public:
	ServerSocket(const uint16_t, std::function<void(ServerSocket*, const uint16_t, const std::string)>);
	ServerSocket() = delete;
	ServerSocket(const ServerSocket&) = delete;
	ServerSocket(ServerSocket&&) = delete;
	ServerSocket& operator=(const ServerSocket&) = delete;
	ServerSocket& operator=(ServerSocket&&) = delete;
	~ServerSocket();

	bool ServInit();
	bool ServRun();
	bool ServSend(const uint32_t, const std::string&);
	bool ServClose();
private:
	std::string ipv4addr;
	uint16_t port;
	std::function<void(ServerSocket*, const uint16_t, const std::string)> msgCallbackFunction;
#ifdef _WIN32
	SOCKET ServMakeSocket();
	SOCKET ServAccept(const SOCKET);
#endif

	// TODO: Linux variants of ServMakeSocket and ServListen
};

#endif 