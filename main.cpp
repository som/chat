#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>

#include <string>

void error(const char *msg, ...){
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);

    perror("\n");
    exit(1);
}

class SocketBase{
protected:
    int _sock_fd;
    sockaddr_in _serv_addr;

public:
    SocketBase():_sock_fd( -1 ){
        std::cout << "SocketBase ctr\n";
    }

    ~SocketBase(){
        close();
    }

    virtual bool init(int portno){
        _sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sock_fd < 0) return false;

        memset(&_serv_addr, 0, sizeof(_serv_addr));
        _serv_addr.sin_family = AF_INET;
        _serv_addr.sin_port = htons(portno);
        return true;
    }

    static void close(int& fd){
        if (fd != -1) {
            if (::close(fd))
                error("Error while shutdown socket");
            fd = -1;
        }
    }

    virtual void close(){
        printf("SocketBase::close %d\n", _sock_fd);
        if (_sock_fd != -1) {
            close(_sock_fd);
        }
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

    static int findHandleForRead(int cnt, ...){
        pollfd apollfd[cnt];
        memset(apollfd, 0, sizeof(apollfd));

        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100;

        va_list ap;
        va_start(ap, cnt);
        for(int i=0; i < cnt; i++){
            apollfd[i].fd = va_arg(ap, int);
            apollfd[i].events = POLLIN;
        }
        va_end(ap);

        int retval = poll(apollfd, cnt, -1);
        if (retval > 0) {
            for (int i = 0; i < cnt; i++) {
                if (apollfd[i].revents & POLLIN)
                    return apollfd[i].fd;
            }
        }

        return -1;
    }

    static std::string read(int fd){
        std::string res;
        const int BUFF_SIZE = 256;
        char buffer[BUFF_SIZE];
        int retval;
        pollfd apollfd = { .fd = fd, .events = POLLIN, .revents = 0 };

        do{
            int n =  fd == ::stdin->_file ? ::read( fd, buffer, BUFF_SIZE - 1 ) : recv(fd, buffer, BUFF_SIZE - 1, 0);
            if (n < 0) error("Error while reading from socket");

            buffer[n] = 0;
            res += buffer;

            apollfd.revents = 0;
            retval = poll(&apollfd, 1, 0);
        }while( retval > 0 && apollfd.revents & POLLIN);

        return res;
    }

    static void write(int fd, const std::string& str){
        const char* buffer = str.c_str();
        int i, len = str.length();
        pollfd apollfd = { .fd = fd, .events = POLLOUT, .revents = 0 };

        for(i = 0; i < len;) {
            apollfd.revents = 0;
            int retval = poll(&apollfd, 1, 0);

            if (retval < 0)
                error("Error while writing to socket");

            if (retval > 0 && apollfd.revents & POLLOUT) {
                int n = send(fd, buffer + i, len - i, 0);
                if (n < 0)
                    error("Error while writing to socket");

                i += n;
            }
        }
    }
};

class Server : public SocketBase{
protected:
    int _rw_sock_fd;
    sockaddr_in cli_addr;

public:
    Server():_rw_sock_fd(-1){
        std::cout << "Server ctr\n";
    }

    virtual bool init(int portno){
        SocketBase::init(portno);

        _serv_addr.sin_addr.s_addr = INADDR_ANY;
        if ( ::bind(_sock_fd, (struct sockaddr *) &_serv_addr, sizeof(_serv_addr)) < 0)
            return false;

        listen(_sock_fd, 5);

        unsigned clilen = sizeof(cli_addr);
        _rw_sock_fd = accept(_sock_fd, (sockaddr *) &cli_addr, &clilen);
        if (_rw_sock_fd < 0)
            return false;

        return true;
    }

    virtual int handle()const{
        return _rw_sock_fd;
    }

    virtual void close(){
        SocketBase::close();
//        printf("Server::close %d\n", _rw_sock_fd);
        SocketBase::close( _rw_sock_fd );
    }
};

class Client : public SocketBase{
protected:
    hostent *server;

public:

    virtual bool init(const char* servername, int portno){
        SocketBase::init( portno );

        server = gethostbyname(servername);
        if (server == NULL) return false;

        bcopy((char *)server->h_addr,
              (char *)&_serv_addr.sin_addr.s_addr,
              server->h_length);

        return (connect(_sock_fd, const_cast<const sockaddr *>( reinterpret_cast<sockaddr *>(&_serv_addr) ), sizeof(_serv_addr))  == 0);
    }
};


int main(int argc, char** argv) {
    static const int PORT = 8080;

    SocketBase* socket;
    Server server;
    Client client;
    if (client.init( "localhost", PORT )) {
        socket = &client;
    }else{

        if (!server.init( PORT ))
            error("Can't open server");

        if (!client.init( "localhost", PORT ))
            error( "Can't connect client" );

        socket = &server;
    }

    std::cout << "Start chat:\n";

    for(bool quit = false; !quit ;){
        int fd = SocketBase::findHandleForRead(2, ::stdin->_file, socket->handle());
        if (fd < 0) continue;

        std::string str = SocketBase::read( fd );
        quit = str == "quit\n";

        if (fd == socket->handle()){
            std::cout << str;
        }else{
            socket->write( str );
        }
    };

    return 0;
}