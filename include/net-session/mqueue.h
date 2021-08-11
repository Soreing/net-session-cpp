#ifndef MQUEUE_H
#define MQUEUE_H

#include "cpmutex.h"
#include "cpevent.h"

//Unit of a network message
class Message
{
public:
    int id;         //ID of the message
    int length;     //Length in bytes of the content
    bool trunc;     //Indicator for the message being truncated
    char* content;  //C-String content of the message

public:
    //Create a message from a network stream
    Message(const char* stream, int bytes);
    //Copy constructor to copy the content by value
    Message(const Message& m);

    //Extracts some or all of the content from the message to a buffer
    //If the message was partially extracted, the length and content is updated
    bool read(char* buff, int size, int &bytes);

    //Deallocates the memory for the content
    ~Message();
};

//MessageNode for the Message Queue
struct MessageNode
{   Message msg;
    MessageNode* next;
};

//Linked List style priority queue of messages, mutex locked for thread safety
//The messages' ID is the key for comparisons and priority
class MessageQueue
{
private:
    MessageNode* head;  //First message in the Queue
    mutex mtx;          //Mutext Lock for thread safety
    signal sig;         //Signal that fires when messages get added

public:
    MessageQueue();

    //Deallocates the memory of all messages and sets head to NULL
    void clear();

    //Adds a message to the queue and keeps it ordered by ID
    //Lower IDs have higher priority in the queue
    void add(Message m);
    //Constructs a message and adds it to the queue
    void add(const char* data, int bytes);
    
    //Extracts some data from the message queue
    //If all the message got consumed, it gets deleted
    int  get(char* buffer, int size);

    //Deletes all nodes
    ~MessageQueue();
};

#endif
