
#ifndef CHAT_MYSOCKET_H
#define CHAT_MYSOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <memory>

#include "helper.h"
#include "ISocket.h"

class SocketBase: public ISocketBase{
protected:
    int _sock_fd = 0;

public:
    SocketBase():_sock_fd( -1 ){
        tr("SocketBase ctr");
    }

    virtual ~SocketBase() = 0;

    virtual bool init(){
        _sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        return _sock_fd >= 0;
    }

    static void close(int& fd);

    virtual void close(){
        if (_sock_fd != -1)
            close(_sock_fd);
    }

    virtual int handle()const{
        return _sock_fd;
    }

    std::string read(){
        return read( handle() );
    }

    void write(const std::string& str){
        write( handle(), str );
    }

    static std::string read(int fd);
    static void write(int fd, const std::string& str);
};

class Server : public SocketBase, public ISocketServer{
protected:
    int _rw_sock_fd = 0;

public:
    Server():_rw_sock_fd(-1){
        tr("Server ctr");
    }

    virtual ~Server(){
        tr("~Server");
        close();
    }

    bool init(int portno);

    virtual int handle()const{
        return _rw_sock_fd;
    }

    virtual void close(){
        if (_rw_sock_fd != -1)
            SocketBase::close( _rw_sock_fd );
    }

    virtual std::string read(){
        return SocketBase::read();
    }

    virtual void write(const std::string& str){
        SocketBase::write(str);
    }
};

class Client : public SocketBase, public ISocketClient{
protected:
public:
    Client(){
        tr("Client ctr");
    }

    virtual ~Client(){
        tr("~Client");
    }

    virtual bool init(const char* servername, int portno){
        if (!SocketBase::init()) return false;

        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        addrinfo *result;
        int r = getaddrinfo(servername, std::to_string( portno ).c_str(), &hints, &result);
        if (r != 0)
            return false;

        addrinfo *rp = result;
        do{
            r = connect(_sock_fd, rp->ai_addr, rp->ai_addrlen);
            rp = rp->ai_next;
        }while(rp && r != 0);
        freeaddrinfo(result);

        return (r == 0);
    }

    virtual std::string read(){
        return SocketBase::read();
    }

    virtual void write(const std::string& str){
        SocketBase::write(str);
    }
    virtual int handle() const{
        return SocketBase::handle();
    }
};


#endif //CHAT_MYSOCKET_H
