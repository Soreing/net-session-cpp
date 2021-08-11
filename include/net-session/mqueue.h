#ifndef MQUEUE_H
#define MQUEUE_H

#include "cpmutex.h"
#include "cpevent.h"
#include <string.h>

class Message
{
public:
    int id;
    int length;
    bool trunc;
    char* content;

public:
    Message(const char* stream, int bytes);
    Message(const Message& m);

    bool read(char* buff, int size, int &bytes);

    ~Message();
};

struct MessageNode
{   Message msg;
    MessageNode* next;
};

class MessageQueue
{
private:
    MessageNode* head;
    mutex mtx;
    signal sig;

public:
    MessageQueue();

    void clear();
    void add(Message m);
    void add(const char* data, int bytes);
    int  get(char* buffer, int size);

    ~MessageQueue();
};

#endif
