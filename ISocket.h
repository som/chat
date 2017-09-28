//
// Created by som on 27/09/2017.
//

#ifndef CHAT_ISOCKET_H
#define CHAT_ISOCKET_H

#include <string>

class ISocketBase{
public:
    virtual std::string read() = 0;
    virtual void write(const std::string& str) = 0;
    virtual int handle() const = 0;
};


class ISocketServer: public virtual ISocketBase{
public:
    virtual bool init(int portno) = 0;
};

class ISocketClient: public virtual ISocketBase{
public:
    virtual bool init(const char* servername, int portno) = 0;
};

#endif //CHAT_ISOCKET_H
