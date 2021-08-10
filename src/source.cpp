#include <net-session/udpsocket.h>
#include <iostream>

//#define CLIENT
#define SERVER

#define BUFFER_SIZE 256
void main()
{

#if defined _WIN32 || defined _WIN64
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{	printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
#endif

#ifdef SERVER
	UDPSocket sock;
	struct sockaddr_in host, peer;
	host.sin_family = AF_INET;
	host.sin_addr.s_addr = INADDR_ANY;
	host.sin_port = htons(8888);

	sock.socket(SOCK_DGRAM);
	sock.bind(&host);

	char buffer[BUFFER_SIZE];	
	while(true)
	{	sock.recvfrom(buffer, BUFFER_SIZE, &peer);
		std::cout<< buffer << "\n";
	}
#endif

#ifdef CLIENT
	UDPSocket sock;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(8888);

	sock.socket(SOCK_DGRAM);
	while(true)
	{	sock.sendto("Hello", 6, &server);
		Sleep(1000);
	}
#endif
}
