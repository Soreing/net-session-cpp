#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "cplatforms.h"
#include "cpmutex.h"
#include "sockhdr.h"

#if defined PLATFORM_WINDOWS
	#include <windows.h>
#elif defined PLATFORM_UNIX
	#include <netinet/in.h>
	typedef int SOCKET;
#endif

//A Cross platform encapsulation of sockets with UDP protocol, datagram or raw socket type
//Raw sockets allow for more customization and get more detail from packets
//Customization includes creating the packet headers instead of the Network and Transport Layer
class UDPSocket
{
private:
	u_long rseed;		//Random Seed for port and packet ID

	IPV4_HDR ihdr;		//IP Header object
	UDP_HDR  uhdr;		//UDP Header object
	SOCKET   sock;		//Network Socket 
	mutex    mtx;		//Mutex Lock

	bool    raw;		//Whether the socket is normal or raw
	u_short inPort;		//Internal Port	( resolved if left 0 )
	u_short packID;		//Packet ID		( resolved if left 0 )
	u_long  locAddr;	//Local address	( resolved if left 0 )

public:
	//Default constructor for UDPSocket
	//Does not create the socket, just initializes it
	UDPSocket();

	//Creates a new UDPSocket
	//Returns the socket's value on success, -1 on failure
	int socket(int type);

	//Binds a socket to a port to listen
	//Returns 0 on success, -1 on error
	int bind(struct sockaddr_in* addr);

	//Sends data through the socket to the address specified
	//Returns the number of bytes sent. -1 is returned on error
	int sendto(const char* buffer, int length, struct sockaddr_in* addr);

	//Receives Data on the socket and stores the return address
	//Returns the number of bytes read. -1 is returned on error
	int recvfrom(char* buffer, int length, struct sockaddr_in* addr);

	//Closes the socket and sets every value but the random seed to 0
	void close();

private:
	//Random number generator using XOR shift
	unsigned long rsock_x_rand();

	//Identifies the host's preferred DHCP Address
	//Returns the address in network byte order. 
	//It returns -1 if the host has no name,
	unsigned long getSourceAddr(unsigned int addr);
};

#endif