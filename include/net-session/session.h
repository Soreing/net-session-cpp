#ifndef SESSION_H
#define SESSION_H

#include "cplatforms.h"
#include "cpevent.h"
#include "udpsocket.h"
#include "mqueue.h"

enum ConnState { Closed = 1, Connecting = 2, Connected=3 };

//Session is a cross platform encapsulation of C sockets to create a live connection between peers using datagrams
//Main functionality of the class is to expedite and maintain UDP connections and queue messages
class Session
{
private:
	struct sockaddr_in peer;	//Address of the remote peer
	UDPSocket ssock;			//Socket ofthe communication
	bool raw;

	long long lastRecv;			//Unix timestamp of the last successful packet received
	int refreshFreq;			//Frequency of sending a keepalive packet in seconds
	int maxTimeout;				//Time in seconds after the application times out
	ConnState state;			//State of the connection (connected or not)

	void* thrKeepalive;		    //Thread that keeps alive the connection
	void* thrReceive;			//Thread that receives data from the peer
	signal sigConn;				//Signal to alert other threads that connecting state is over
	mutex  mtx;					//Mutex lock to guarantee thread safety

	int msgID;					//ID number of the message being sent (increments)
	MessageQueue* queue;		//Message Queue for storing messages	

public:
	//Create a new session object with default parameters
	//Optional parameter "raw" indicates usage of SOCK_RAW instead of DGRAM
	Session(bool raw = false);

	//Creates an address info structure from the port and host address
	//Attempts to connect to the address. Returns -1 if no address found
	int connect(int port, const char* addr);

	//Disconnects any previous connection and connects to a new address
	//Opens the communication, then the keepalive and receiving threads are started
	//Waits till the receiving thread confirms a connection or times out
	//Returns 0 if sucessfully connected or -1 on error
	int connect(struct sockaddr_in addr);

	//Terminates all active threads and closes the socket
	//Clears the message queue and sets the state to closed
	void disconnect();

	//Sends a stream of bytes over the socket
	int send(const char* msg, int length);

	//Extracts information from the message queue
	int recv(char* buf, int size);

	//Getters and Setters
	inline ConnState getState() {return state;}

	//Disconencts any active connection and deletes the message queue
	~Session();

	//Thread friend declarations
	#if defined PLATFORM_WINDOWS
		friend DWORD WINAPI keepalive(void* lparam);
		friend DWORD WINAPI receive(void* lparam);
	#elif defined PLATFORM_UNIX
		friend void* keepalive(void* lparam);
		friend void* receive(void* lparam);
	#endif
};


//Interface wrapper for Sessions
//Use this class for interacting with a Session object
class ISession
{
private:
	Session* ssn;		//Pointer to the session

public:
	inline ISession(bool raw = false) : ssn(new Session(raw)) { }

	inline int connect(int port, const char* addr) { return ssn->connect(port, addr); }
	inline int connect(struct sockaddr_in addr)    { return ssn->connect(addr); }
	
	inline void disconnect() { ssn->disconnect(); }
	
	inline int send(const char* msg, int length) { return ssn->send(msg, length); }
	inline int recv(char* buf, int size) { return ssn->recv(buf, size); }

	inline void close() { delete ssn; }

	inline ConnState getState() { return ssn->getState(); }
};
#endif