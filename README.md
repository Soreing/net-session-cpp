# net-session-cpp

# Installation
Add the folder `net-session` from `/include` in your include path. If you want to compile the library from source, add all `.cpp` files but `source.cpp` from the `/src` folder. Alternatively, you can compile the source code to a static library and include it that way.

# Description
net-session-cpp is a Windows/Linux cross platform C++ library that establishes and maintains a network connection between peers using UDP protocol. After the connection has been established, the session sends keepalive packets to maintain the connection and listens to messages that get stored in a message queue.

# Basic Usage
Sessions are created through an interface class to ensure the management of the resources is not disrupted. You can create an `ISession` object, which you will need to `close()` when you are finished to avoid memory leaks.  
  
Once you have created a session, you can use the `connect()` function with a **port number** and an **IP address** to connect to a remote host. The function returns 0 on success or -1 on error. You can `disconnect()` a socket manually if you want to close the connection without deleting the session 
```c++
ISession ssn;
if(ssn.connect(59000, "127.0.0.1") == 0)
{   cout << "Connected to the remote host\n";
}
ssn.disconnect();
ssn.close()
```
While the session is connected, you can send or receive data with the `send()` and `recv()` functions. You can send **at most 1024 bytes** of data at once, the rest will get truncated on the peer's side. Both functions return the number of bytes that were sent or received, or -1 on error.
```c++
int bytes;
string str;
char buffer[1024];

while (ssn.getState() == Connected)
{   
    getline(cin, str);
    ssn.send(str.c_str(), str.length()+1);

    bytes = ssn.recv(buffer, 1024);
    if(bytes > 0)
    {   cout << buffer <<"\n";
    }
}
```
# UDP Sockets
Sessions use a cross platform encapsulation of native C sockets implemented with UDP protocol in mind. You can use UDP Sockets to create a server listening for connections relatively easily. You need to have the following:
- Create a UDP Socket
- Create an address structure
- Bind the socket to the address
- Start receiving messages

```c++
UDPSocket sock;
sock.socket(SOCK_DGRAM);

struct sockaddr_in host, peer;
host.sin_family = AF_INET;
host.sin_addr.s_addr = INADDR_ANY;
host.sin_port = htons(PORT);

sock.bind(&host);

int bytes;
char buffer[1024];
bytes = sock.recvfrom(buffer, 1024, &peer);
```
UDPSockets implement the `sendto()` and `recvfrom()` functions, which in addition to sessions' `send()` and `recv()` functions, require an address structure to send to or receive messages from.  
  
UDP sockets can be disconnected with the `close()` function.

# Additional Details
## P2P Connections
Sessions implement another `connect()` function for connecting to a remote peer. Instead of supplying a port and an IP address, it takes an address structure you can receive by a server mediating the connection. The keepalive packets will perform UDP hole punching and keep the reply address open.

## Raw sockets and sessions
Creating an `ISession` with a bool parameter true will switch the socket from `SOCK_DGRAM` to `SOCK_RAW`. Raw sockets add the IPv4 and UDP headers in the application instead of the Operating System. UDPSockets can also be created with `SOCK_RAW` type instead of `SOCK_DGRAM`.  
**Raw sockets require admin/root privileges to operate.**
```c++
//Session using raw socket
ISession ssn(true);

//UDPSocket using raw Socket
UDPSocket sock;
sock.socket(SOCK_DGRAM);
```