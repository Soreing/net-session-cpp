#include <net-session/mqueue.h>
#include <iostream>

//Create a message from a network stream
Message::Message(const char* stream, int bytes)
{
    id      = *(int*)(stream+1);
    length  = *(int*)(stream+5);
    trunc   = false;
    content = NULL;

    if(length > bytes-9)
    {   trunc = true;
        length = bytes-9;
    }
    
    content = new char[length];
    memcpy(content, stream+9, length);
}

//Copy constructor to copy the content by value
Message::Message(const Message& m)
{
    id      = m.id;
    length  = m.length;
    trunc   = m.trunc;

    content = new char[length];
    memcpy(content, m.content, length);
}

//Extracts some or all of the content from the message to a buffer
    //If the message was partially extracted, the length and content is updated
bool Message::read(char* buff, int size, int &bytes)
{
    if(size >= length)
    {   memcpy(buff, content, length);
        bytes = length;

        return true;
    }
    else
    {   char* remainder = new char[length-size];
        memcpy(buff, content, size);
        memcpy(remainder, content+size, length-size);
        bytes = size;

        delete[] content;
        content = remainder;
        length -= size;
        return false;
    }
}

//Deallocates the memory for the content
Message::~Message()
{
    if(content != NULL)
    {   delete[] content;
    }
}


MessageQueue::MessageQueue() : head(NULL)
{
}

//Deallocates the memory of all messages and sets head to NULL
void MessageQueue::clear()
{
    mtx.lock();
    MessageNode* nxt = NULL;
    MessageNode* cur = head;
    head = NULL;

    while(cur != NULL)
    {   nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    mtx.unlock();
}

//Adds a message to the queue and keeps it ordered by ID
//Lower IDs have higher priority in the queue
void MessageQueue::add(Message m)
{
    mtx.lock();

    MessageNode* cur = head;
    if(cur == NULL)
    {   head = new MessageNode{m, NULL};
    }
    else if(cur->msg.id > m.id)
    {   head = new MessageNode{m, cur};
    }
    else
    {   while (cur->next != NULL && cur->next->msg.id < m.id)
        {   cur = cur->next;
        }
        cur->next = new MessageNode{m, cur->next};
    }

    sig.set();
    mtx.unlock();
}

//Constructs a message and adds it to the queue
void MessageQueue::add(const char* data, int bytes)
{
    Message m(data, bytes);
    add(m);
}

//Extracts some data from the message queue
//If all the message got consumed, it gets deleted
int MessageQueue::get(char* buffer, int size)
{
    MessageNode* nxt = NULL;
    bool consumed;
    int bytes;
    
    while(true)
    {   
        if(head == NULL)
        {    sig.wait();
        }
        
        mtx.lock();

        if(head != NULL)
        {   consumed = head->msg.read(buffer, size, bytes);
            
            if(consumed)
            {   nxt = head->next;
                delete head;
                head = nxt;
            }

            mtx.unlock();
            return bytes;
        }

        mtx.unlock();
    } 
}

//Deletes all nodes
MessageQueue::~MessageQueue()
{
    clear();
}

