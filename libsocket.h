//
// Created by som on 26/09/2017.
//

#ifndef CHAT_LIBSOCKET_H
#define CHAT_LIBSOCKET_H

#include "ISocket.h"
#include "inetserverstream.hpp"
#include "socket.hpp"
#include "select.hpp"


class LSServer: public ISocketServer{
protected:
    libsocket::inet_stream_server _srv;
    std::unique_ptr<libsocket::inet_stream> _client;

public:
    LSServer(int portno){
        init( portno );
    }
    virtual bool init(int portno);
    virtual std::string read();
    virtual void write(const std::string& str);
    virtual int handle() const{
        return _client->getfd();
    }
};


class LSClient: public ISocketClient{
protected:
    libsocket::inet_stream _client;

public:
    virtual bool init(const char* servername, int portno);
    virtual std::string read();
    virtual void write(const std::string& str);
    virtual int handle() const{
        return _client.getfd();
    }
};
#endif //CHAT_LIBSOCKET_H
