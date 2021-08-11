#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <net-session/cplatforms.h>
#include <net-session/udpsocket.h>
#include <string.h>
#include <time.h>

#define LOCALHOST 16777343	//Network byte order address of 127.0.0.1

#if defined PLATFORM_WINDOWS
	#define IP_HDRINCL 2
	typedef int socklen_t;
#endif

//Default constructor for UDPSocket
//Does not create the socket, just initializes it
UDPSocket::UDPSocket() 
	:	rseed((u_long)time(NULL)), sock(0), raw(false), 
		inPort(0), packID(0), locAddr(0)
{
}

//Creates a new udp socket
//Returns the socket's value on success, -1 on failure
int UDPSocket::socket(int type)
{
	int hdrOpt = 1;
	close();

	mtx.lock();
	if(type == SOCK_RAW)
	{	sock = ::socket(AF_INET, type, IPPROTO_UDP);
		setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *)&hdrOpt, sizeof(hdrOpt));
		raw = true;
	}
	else if(type == SOCK_DGRAM)
	{	sock = ::socket(AF_INET, type, IPPROTO_UDP);
	}
	mtx.unlock();

	return sock == 0 ? -1 : (int)sock;
}


int UDPSocket::bind(struct sockaddr_in* addr)
{
	inPort  = ntohs(addr->sin_port);
	return ::bind(sock, (struct sockaddr*)addr, sizeof(*addr));
}


//Sends data through the socket to the address specified
//Returns the number of bytes sent. -1 is returned on error
int UDPSocket::sendto(const char* buffer, int length, struct sockaddr_in* addr)
{
	int sentBytes = 0;
	mtx.lock();

	if(!raw)
	{	sentBytes = ::sendto(sock, buffer, length, 0, (struct sockaddr*)addr, sizeof(*addr));
	}
	else
	{	int packetlen = length + sizeof(IPV4_HDR) + sizeof(UDP_HDR);
		char* packet  = new char[packetlen];
		
		if (packID  == 0) packID  = (unsigned short) (32768 + rsock_x_rand() % 32768);
		if (inPort  == 0) inPort  = (unsigned short) (32768 + rsock_x_rand() % 32768);
		if (locAddr == 0) locAddr = getSourceAddr(addr->sin_addr.s_addr);

		if (locAddr == ~0)
		{	return -1;
		}

		ihdr.setAddress(addr->sin_addr.s_addr, locAddr)
			.setProtocol(IPPROTO_UDP)
			.setID(packID)
			.checksum(packetlen)
			.load(packet);
		
		uhdr.setPort(inPort, ntohs(addr->sin_port))
			.setLength( (u_short)(packetlen-sizeof(IPV4_HDR)) )
			.load(packet+sizeof(IPV4_HDR));

		memcpy(packet + sizeof(IPV4_HDR) + sizeof(UDP_HDR), buffer, length);
		sentBytes = ::sendto(sock, packet, packetlen, 0, (struct sockaddr*)addr, sizeof(*addr));
		delete[] packet;
	}

	mtx.unlock();
	return sentBytes;
}


//Receives Data on the socket and stores the return address
//Returns the number of bytes read. -1 is returned on error
int UDPSocket::recvfrom(char* buffer, int length, struct sockaddr_in* addr)
{
	IPV4_HDR* ipheader;
	UDP_HDR*  udpHeader;
	int allBytes = 0;
	int datBytes = 0;

	int arrdLen = sizeof(struct sockaddr);

	for (; sock > 0;)
	{
		if(!raw)
		{	allBytes  = ::recvfrom(sock, buffer, length, 0, (struct sockaddr*)addr, (socklen_t*)&arrdLen);
			return allBytes;
		}
		else
		{	allBytes  = ::recvfrom(sock, buffer, length, 0, NULL, NULL);
			ipheader  = (IPV4_HDR*)buffer;

			if (allBytes < 0 || ntohs(ipheader->ip_length) != allBytes)
			{	return -1;
			}

			udpHeader = (UDP_HDR*)(buffer + (4 * ipheader->ip_hl));
			datBytes  = ntohs(udpHeader->length) - sizeof(UDP_HDR);

			mtx.lock();
			if (udpHeader->dstport == htons(inPort))
			{
				if (datBytes > length)
				{	return -1;
				}

				addr->sin_family = AF_INET;
				addr->sin_port = udpHeader->srcport;
				addr->sin_addr.s_addr = ipheader->ip_srcaddr;
				mtx.unlock();

				memcpy(buffer, buffer + (allBytes-datBytes), datBytes);
				return datBytes;
			}
			mtx.unlock();
		}
	}

	return 0;
}

//Closes the socket and sets every value but the random seed to 0
void UDPSocket::close()
{
	mtx.lock();
	
	#if defined PLATFORM_WINDOWS
		closesocket(sock);
	#elif defined UNIX_PLATOFRM
		close(sock);
	#endif

	raw = false;
	sock = 0;
	inPort = 0;
	packID = 0;

	mtx.unlock();
}

//Random number generator using XOR shift
unsigned long UDPSocket::rsock_x_rand()
{
	rseed ^= (rseed << 13);
	rseed ^= (rseed >> 17);
	rseed ^= (rseed << 5);

	return rseed;
}

//Identifies the host's preferred DHCP Address
//Returns the address in network byte order. It returns -1 if the host has no name,
//-2 if it has no adapters or -3 if not connected to a network
unsigned long UDPSocket::getSourceAddr(unsigned int addr)
{
	int sock = (int)::socket(AF_INET, SOCK_DGRAM, 0);
	int addrlen = sizeof(sockaddr_in);

	sockaddr_in loopback;
	loopback.sin_family = AF_INET;
	loopback.sin_addr.s_addr = addr;
	loopback.sin_port = htons(9);

	if (connect(sock, (struct sockaddr*)&loopback, sizeof(loopback)) == -1)
	{	return -1;
	}

	if (getsockname(sock, (struct sockaddr*)&loopback, (socklen_t*)&addrlen) == -1)
	{	return -1;
	}

	return loopback.sin_addr.s_addr;
}