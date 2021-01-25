#include <iostream>

#include "inc/ServerSocket.h"

void callbackFunction(ServerSocket* serv_sock, SOCKET client_sock, std::string msg)
{
	serv_sock->ServSend(client_sock, msg);
	std::cout << "msg received: " << msg << std::endl;
}

int main()
{
	ServerSocket testServ(10101, callbackFunction);

	if (!testServ.ServInit())
		return -1;

	testServ.ServRun();
	return 0;
}