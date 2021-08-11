#include <net-session/session.h>
#include <iostream>

#define CLIENT
//#define SERVER

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

	char buffer[256];
	sock.recvfrom(buffer, 256, &peer);

	Session ssn;
	ssn.connect(peer);
	while (ssn.state == Connected);
#endif

#ifdef CLIENT
	Session ssn;
	ssn.connect(PORT, IP_ADR);
	while (ssn.state == Connected);
#endif
}