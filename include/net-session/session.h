#ifndef SESSION_H
#define SESSION_H

#include "mqueue.h"
#include "udpsocket.h"
#include "cplatforms.h"
#include "cpevent.h"

#include <iostream>

enum ConnState { Closed = 1, Connecting = 2, Connected=3 };

//Session is a cross platform encapsulation of C sockets to create a live connection between peers using datagrams
//Main functionality of the class is to expedite and maintain UDP connections and queue messages
class Session
{
public:
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
	//Initializes all members of the class and allocates memory for the messageQueue
	//Can throw Session Exceptions if WSA or the socket fails to initialize
	Session(bool raw = false);

	//Closes the session and deletes the messageQueue
	~Session();

	//Creates an address info structure from the port and host address and attempts to connect to it
	//If a connection is made, the keepalive and receiving threads are started
	//Can throw Session Exception if the connection times out, the socket is unable to send/receive 
	//Or if the peer is not accepting connections, or if the host given has no address/can't be found
	int connect(int port, const char* addr);

	//Attempts to connect to the return address information provided
	//If a connection is made, the keepalive and receiving threads are started
	//Can throw Session Exception if the connection times out, the socket is unable to send/receive 
	//Or if the peer is not accepting connections
	int connect(struct sockaddr_in addr);

	//Terminates all active threads and closes the socket
	//The process is mutex locked to prevent threads interacting with the object
	void disconnect();

	//Sends a stream of bytes over the socket
	//Can throw a Session Exception if the session is not not connected
	int send(const char* msg, int length);

	//Extracts information from the message queue
	int recv(char* buf, int size);

};
#endif