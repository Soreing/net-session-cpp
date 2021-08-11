#include <net-session/session.h>
#include <iostream>
#include <string>

//#define CLIENT
#define SERVER

#define PORT 59000
#define IP_ADR "192.168.2.184"

int main()
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
	host.sin_port = htons(PORT);

	sock.socket(SOCK_DGRAM);
	sock.bind(&host);

	char buffer[1024];
	int rv = sock.recvfrom(buffer, 1024, &peer);

	Session ssn;
	ssn.connect(peer);
	std::cout<< "Connection Made!\n";

	std::string str;
	char msg[6] = {0,0,0,0,0,0};
	while (ssn.state == Connected)
	{	ssn.recv(msg, 5);
		std::cout<< msg <<"\n";

	}
#endif

#ifdef CLIENT
	Session ssn;
	ssn.connect(PORT, IP_ADR);
	std::cout<< "Connection Made!\n";

	std::string str;
	while (ssn.state == Connected)
	{	getline(std::cin, str);
		ssn.send(str.c_str(), str.length()+1);
	}
#endif
}