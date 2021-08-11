#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <net-session/session.h>
#include <iostream>
#include <string.h>
#include <string>
#include <time.h>

#define KAL 1		//Flag for keepalive messages
#define MSG 4		//Flag for real messages

#define HDR_SIZE 9	//Message Header Size

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

//Sends Keepalive packets till the connection is closed
//If the socket encounters an error or the session times out, disconnects
THREAD_T keepalive(void* lparam)
{
	Session* ssn  = (Session*)lparam;
	long long now = 0;
	int traffic   = 0;
	char buffer   = KAL;

	while (ssn->state != Closed)
	{
		//Send a keepalive packet and note down the time
		traffic = ssn->ssock.sendto(&buffer, 1, &ssn->peer);
		now = time(NULL);

		ssn->mtx.lock();
		//If the keepalive packet failed or the session timed out, quit
		if (traffic <= 0 || now - ssn->lastRecv > ssn->maxTimeout)
		{	ssn->thrKeepalive = NULL;
			ssn->mtx.unlock();

			ssn->disconnect();
			EXIT_THREAD(-1);
		}
		ssn->mtx.unlock();

		//Sleep till the next keepalive packet
		#if defined PLATFORM_WINDOWS
			Sleep(ssn->state == Connecting ? 1000 : (1000 * ssn->refreshFreq));
		#elif defined PLATFORM_UNIX
			sleep(ssn->state == Connecting ? 1 : ssn->refreshFreq);
		#endif
	}

	ssn->thrKeepalive = NULL;
	EXIT_THREAD(0);
}

//Listens and Receives data from the socket till it's closed
//Keepalive packets refresh the conenction time
//Sets the session to connected on the first packet if it was connecting
//Messages are added to the message queue
//If the socket encounters an error, disconnects
THREAD_T receive(void* lparam)
{
	Session* ssn = (Session*)lparam;
	int  packetID = 0;
	int  traffic  = 0;
	int  promised = 0;
	char buffer[1024];

	while (ssn->state != Closed)
	{
		//Read data from the peer
		traffic = ssn->ssock.recvfrom(buffer, 512, &ssn->peer);

		//If no data was sent or an error occuted, quit
		if (traffic <= 0)
		{	ssn->thrReceive = NULL;
			ssn->disconnect();
			EXIT_THREAD(-1);
		}
		//If the connection was connecting, set it to connected
		else
		{	ssn->mtx.lock();
			ssn->lastRecv = time(NULL);
			if (ssn->state == Connecting)
			{	ssn->state = Connected;
				ssn->sigConn.set();
			}
			ssn->mtx.unlock();
		}
		
		//If it's a message, add itto the message quee
		if(buffer[0] == MSG)
		{	ssn->queue->add(buffer, traffic);
		}

	}

	ssn->thrReceive = NULL;
	EXIT_THREAD(0);
}

//Create a new session object with default parameters
//Optional parameter "raw" indicates usage of SOCK_RAW instead of DGRAM
Session::Session(bool raw) 
	:	thrReceive(NULL), thrKeepalive(NULL), raw(raw),
		lastRecv(0), refreshFreq(5), maxTimeout(5), state(Closed),
		msgID(0), queue(new MessageQueue)
{
	if (ssock.socket(raw ? SOCK_RAW : SOCK_DGRAM) == -1)
	{	throw "Error";
	}
}

//Disconencts any active connection and deletes the message queue
Session::~Session() 
{
	disconnect();  
	delete queue;
}

//Creates an address info structure from the port and host address
//Attempts to connect to the address. Returns -1 if no address found
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

//Disconnects any previous connection and connects to a new address
//Opens the communication, then the keepalive and receiving threads are started
//Waits till the receiving thread confirms a connection or times out
//Returns 0 if sucessfully connected or -1 on error
int Session::connect(struct sockaddr_in addr)
{
	disconnect();

	char packet = KAL;
	lastRecv = time(NULL);
	state = Connecting;

	memcpy(&peer, &addr, sizeof(struct sockaddr_in));

	ssock.sendto(&packet, 1, &peer);
	CREATE_THREAD(thrKeepalive, keepalive, this);
	CREATE_THREAD(thrReceive, receive, this);

	sigConn.wait();
	return state == Connected ? 0 : -1;
}

//Terminates all active threads and closes the socket
//Clears the message queue and sets the state to closed
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
		queue->clear();
	}
	mtx.unlock();
}

//Sends a stream of bytes over the socket
int Session::send(const char* msg, int length)
{
	int sent  = 0;
	char *str = new char[length + 9];

	mtx.lock();

	*(char*)(str + 0) = MSG;
	*(int*)(str + 1)  = msgID;
	*(int*)(str + 5)  = length;
	memcpy(str + 9, msg, length);

	sent = ssock.sendto(str, length + 9, &peer);
	delete[] str;
	msgID++;
	
	if(sent <= 0)
	{	mtx.unlock();
		disconnect();
		return -1;
	}

	mtx.unlock();
	return sent;
}

//Extracts information from the message queue
int Session::recv(char* buf, int size)
{
	return queue->get(buf, size);
}