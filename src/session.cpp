#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <net-session/session.h>
#include <iostream>
#include <string.h>
#include <time.h>

#define KAL 1	//Flag for keepalive messages
#define MSG 4	//Flag for real messages

#if defined PLATFORM_WINDOWS
	#include <processthreadsapi.h>
	#define THREAD_T DWORD WINAPI
	#define CREATE_THREAD(handle, func, args) handle = CreateThread(NULL, NULL, func, args, NULL, NULL)
	#define TERMINATE_THREAD(handle) TerminateThread(handle, 0)
	#define EXIT_THREAD(val) return val
#elif defined PLATFORM_UNIX
	#include <pthread.h>
	#include <signal.h>
	#include <unistd.h>
	#include <netdb.h>

	#define THREAD_T void*
	#define CREATE_THREAD(handle, func, args) pthread_create((pthread_t*)&handle, NULL, func, args)
	#define TERMINATE_THREAD(handle) pthread_kill((pthread_t)handle, 0)
	#define EXIT_THREAD(val) return (void*)val
#endif

THREAD_T keepalive(void* lparam)
{
	Session* ssn  = (Session*)lparam;
	long long now = 0;
	int traffic   = 0;
	char buffer   = KAL;

	while (ssn->state != Closed)
	{
		// Send a keepalive packet and note down the time
		traffic = ssn->ssock.sendto(&buffer, 1, &ssn->peer);
		now = time(NULL);

		ssn->mtx.lock();
		// If the keepalife packet failed or the session timed out, quit
		if (traffic <= 0 || now - ssn->lastRecv > ssn->maxTimeout)
		{
			std::cout<< "Session Failed In Keepalive...\n";
			ssn->thrKeepalive = NULL;
			ssn->mtx.unlock();

			ssn->disconnect();
			EXIT_THREAD(-1);
		}
		ssn->mtx.unlock();

		// Sleep till the next keepalive packet
		#if defined PLATFORM_WINDOWS
			Sleep(ssn->state == Connecting ? 1000 : (1000 * ssn->refreshFreq));
		#elif defined PLATFORM_UNIX
			sleep(ssn->state == Connecting ? 1 : ssn->refreshFreq);
		#endif
	}

	ssn->mtx.lock();
	ssn->thrKeepalive = NULL;
	ssn->mtx.unlock();
	std::cout<< "Keepalive Quit...\n";
	EXIT_THREAD(0);
}

THREAD_T receive(void* lparam)
{
	Session* ssn = (Session*)lparam;
	int  packetID = 0;
	int  traffic  = 0;
	int  promised = 0;
	char buffer[512];

	while (ssn->state != Closed)
	{
		// Read data from the peer
		std::cout<< "Receiving Data from Peer...\n";
		traffic = ssn->ssock.recvfrom(buffer, 512, &ssn->peer);

		// If no data was sent or an error occuted, quit
		if (traffic <= 0)
		{
			std::cout<< "Session Failed In Receive...\n";
			ssn->thrReceive = NULL;
			ssn->disconnect();
			EXIT_THREAD(-1);
		}
		// If the connection was connecting, set it to connected
		else
		{	ssn->mtx.lock();
			ssn->lastRecv = time(NULL);
			if (ssn->state == Connecting)
			{	ssn->state = Connected;
				ssn->sigConn.set();
			}
			ssn->mtx.unlock();
		}
		
		// If it's a message
		if(buffer[0] == MSG)
		{	
		}

	}

	ssn->mtx.lock();
	ssn->thrReceive = NULL;
	ssn->mtx.unlock();
	std::cout<< "Receive Quit...\n";
	EXIT_THREAD(0);
}


Session::Session() 
	:	thrReceive(NULL), thrKeepalive(NULL), 
		lastRecv(0), refreshFreq(5), maxTimeout(5), state(Closed)
{
	if (ssock.socket(SOCK_DGRAM) == -1)
	{	throw "Error";
	}
}

Session::~Session() 
{
	disconnect();  
}

int Session::connect(int port, const char* hostname)
{
	hostent *hostIP;
	struct sockaddr_in addr;
	memset((char *)&addr, 0, sizeof(addr));

	if ((hostIP = gethostbyname(hostname)) == NULL)
	{	return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = *(unsigned long*)(hostIP->h_addr_list[0]);

	return connect(addr);
}

int Session::connect(struct sockaddr_in addr)
{
	char packet = KAL;
	lastRecv = time(NULL);
	state = Connecting;

	memcpy(&peer, &addr, sizeof(struct sockaddr_in));

	ssock.sendto(&packet, 1, &peer);
	CREATE_THREAD(thrKeepalive, keepalive, this);
	CREATE_THREAD(thrReceive, receive, this);

	sigConn.wait();
	return state == Connected ? 1 : 0;
}


void Session::disconnect()
{
	mtx.lock();
	
	if(state != Closed)
	{	
		if (thrReceive != NULL) 
		{	TERMINATE_THREAD(thrReceive);
			thrReceive   = NULL;
		}

		if (thrKeepalive != NULL) 
		{	TERMINATE_THREAD(thrKeepalive);
			thrKeepalive = NULL;
		}

		ssock.close();

		if(state == Connecting)
		{	sigConn.set();
		}

		state = Closed;
		std::cout<< "Disconnected...\n";
	}
	mtx.unlock();
}
