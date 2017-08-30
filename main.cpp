#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include <string>
#include <stdio.h>

//using namespace std;

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

    virtual void init(int portno){
        _sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sock_fd < 0)
            error("ERROR opening socket");

        memset(&_serv_addr, 0, sizeof(_serv_addr));
        _serv_addr.sin_family = AF_INET;
        _serv_addr.sin_port = htons(portno);
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

    static std::string read(int fd){
        std::string res;
        const int BUFF_SIZE = 256;
        char buffer[BUFF_SIZE];
        int retval;
        fd_set rfds;

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        do{
            int n =  fd == ::stdin->_file ? ::read( fd, buffer, BUFF_SIZE-1 ) : recv(fd, buffer, BUFF_SIZE-1, 0);
            if (n < 0) error("Error while reading from socket");
            buffer[n] = 0;

            res += buffer;

            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);

            retval = select(1, &rfds, NULL, NULL, &tv);

        }while( retval > 0 && FD_ISSET(fd, &rfds) );

        return res;
    }

    static void write(int fd, const std::string& str){
        const char* buffer = str.c_str();
        int i, len = str.length();

        for(i = 0; i < len;) {
            int n = send(fd, buffer + i, len - i, 0);
            if (n < 0)
                error("Error while writing to socket");

            i += n;
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

    virtual void init(int portno){
        SocketBase::init(portno);

        _serv_addr.sin_addr.s_addr = INADDR_ANY;
        if ( ::bind(_sock_fd, (struct sockaddr *) &_serv_addr, sizeof(_serv_addr)) < 0)
            error("ERROR on binding");

        listen(_sock_fd, 5);

        unsigned clilen = sizeof(cli_addr);
        _rw_sock_fd = accept(_sock_fd, (sockaddr *) &cli_addr, &clilen);
        if (_rw_sock_fd < 0)
            error("ERROR on accept");
    }

    void doIt(){
        std::string s = read( _rw_sock_fd );
        printf("Here is the message:\n%s\n", s.c_str());

        write( _rw_sock_fd, "I got your message" );
    }

    virtual void close(){
        SocketBase::close();
        printf("Server::close %d\n", _rw_sock_fd);
        SocketBase::close( _rw_sock_fd );
    }
};

class Client : public SocketBase{
protected:
    hostent *server;

public:

    virtual void init(char* servername, int portno){
        SocketBase::init( portno );

        server = gethostbyname(servername);
        if (server == NULL)
            error( "ERROR, no such host: %s", servername );

        bcopy((char *)server->h_addr,
              (char *)&_serv_addr.sin_addr.s_addr,
              server->h_length);

        if (connect(_sock_fd, const_cast<const sockaddr *>( reinterpret_cast<sockaddr *>(&_serv_addr) ), sizeof(_serv_addr)) < 0)
            error("ERROR connecting");
    }

    void doIt(){
        std::cout << "Please enter the message:\n";
        std::string s = read( ::stdin->_file );
        write( _sock_fd, s );

        s = read(_sock_fd);
        std::cout << s;
    }

};

int main(int argc, char** argv) {
    if (argc == 3 && *argv[1] == 's'){
        Server server;
        server.init( atoi(argv[2]) );
        server.doIt();
    }else if (argc == 4 && *argv[1] == 'c'){
        Client client;
        client.init( argv[2], atoi(argv[3]) );
        client.doIt();
    }else std::cout << "Usage:\nchat s port\nchat c host port";


    return 0;
}