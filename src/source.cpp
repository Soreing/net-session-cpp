#include <net-session/session.h>
#include <iostream>
#include <string>

//#define CLIENT
#define SERVER

#define BUF_LEN 1024
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
	ISession ssn;

	char buffer[BUF_LEN];
	int bytes;

	struct sockaddr_in host, peer;
	host.sin_family = AF_INET;
	host.sin_addr.s_addr = INADDR_ANY;
	host.sin_port = htons(PORT);

	sock.socket(SOCK_DGRAM);
	sock.bind(&host);

	bytes = sock.recvfrom(buffer, BUF_LEN, &peer);
	if(bytes > 0)
	{	std::cout<< "Connection Made!\n";
		ssn.connect(peer);

		std::string str;
		while (ssn.getState() == Connected)
		{	bytes = ssn.recv(buffer, BUF_LEN);
			if(bytes > 0)
			{	ssn.send(buffer, bytes+1);
			}
		}
	}

	ssn.close();
#endif

#ifdef CLIENT
	ISession ssn;

	char buffer[BUF_LEN];
	int bytes;

	if( ssn.connect(PORT, IP_ADR) == 0 )
	{	std::cout<< "Connection Made!\n";
	}

	std::string str;
	while (ssn.getState() == Connected)
	{	std::cout<< "<Client> ";
		getline(std::cin, str);

		ssn.send(str.c_str(), str.length()+1);
		bytes = ssn.recv(buffer, BUF_LEN);

		if(bytes > 0)
		{	std::cout<< "<Server> " << buffer <<"\n";
		}
	}

	ssn.close();
#endif
}