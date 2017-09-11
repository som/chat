#include <iostream>
#include <memory>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>


void error(const char *msg, ...){
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);

    perror("\n");
    exit(1);
}

void tr( const char* msg){
//     std::cout << msg << std::endl;
}

void tr( const char* msg, int i){
//     std::cout << msg << i << std::endl;
}

class SocketBase{
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

    static void close(int& fd){
        tr("SocketBase::close ", fd);
        if (fd != -1) {
            shutdown( fd, SHUT_WR );
            shutdown( fd, SHUT_RD );
            if (::close(fd))
                error("Error while close socket");
            fd = -1;
        }
    }

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

    template < int size >
    static int findHandleForRead(const int (&afd)[ size ]){
        pollfd apollfd[size];
        memset(apollfd, 0, sizeof(apollfd));
        timeval tv = { .tv_sec = 0, .tv_usec = 100 };

        for(int i=0; i < size; i++){
            apollfd[i].fd = afd[i];
            apollfd[i].events = POLLIN;
        }

        int retval = poll(apollfd, size, -1);
        if (retval > 0) {
            for (int i = 0; i < size; i++) {
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
            ssize_t n =  fd == ::stdin->_file ? ::read( fd, buffer, BUFF_SIZE - 1 ) : recv(fd, buffer, BUFF_SIZE - 1, 0);
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
        unsigned long i, len = str.length();
        pollfd apollfd = { .fd = fd, .events = POLLOUT, .revents = 0 };

        for(i = 0; i < len;) {
            apollfd.revents = 0;
            int retval = poll(&apollfd, 1, 0);
            if (retval < 0)
                error("Error while writing to socket");

            if (retval > 0 && apollfd.revents & POLLOUT) {
                ssize_t n = send(fd, buffer + i, len - i, 0);
                if (n < 0)
                    error("Error while writing to socket");

                i += n;
            }
        }
    }
};

SocketBase::~SocketBase() {
    std::cout << "~SocketBase\n";
    close();
}

class Server : public SocketBase{
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

    bool init(int portno){
        if (!SocketBase::init()) return false;
        int option = 1;
        if (setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1)
            error("Error on reuse");

        std::cout << "Bind...\n";
        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(portno);
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        if ( bind(_sock_fd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0 )
            return false;

        std::cout << "Listen...\n";
        listen(_sock_fd, 5);

        sockaddr_in cli_addr;
        unsigned clilen = sizeof(cli_addr);
        std::cout << "Accept...\n";
        _rw_sock_fd = accept(_sock_fd, reinterpret_cast<sockaddr*>(&cli_addr), &clilen);
        return _rw_sock_fd >= 0;
    }

    virtual int handle()const{
        return _rw_sock_fd;
    }

    virtual void close(){
        if (_rw_sock_fd != -1)
            SocketBase::close( _rw_sock_fd );
    }
};

class Client : public SocketBase{
protected:
public:
    Client(){
        tr("Client ctr");
    }

    ~Client(){
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
};

SocketBase* findSocket() throw(){
    static const int PORT = 8080;
    Client* client = new Client();
    if (client->init("localhost", PORT)) return client;
    delete client;

    Server* server = new Server();
    if (!server->init( PORT )){
        delete server;
        error("Can't open server");
    }

    return server;
}

void chat(){
    std::auto_ptr<SocketBase> socket( findSocket() );
    std::cout << "Start chat:\n";

    int afd[] = { ::stdin->_file, socket->handle() };
    for(bool quit = false; !quit ;){
        int fd = SocketBase::findHandleForRead( afd );
        if (fd < 0) continue;

        std::string str = SocketBase::read( fd );
        quit = str == "quit\n";

        if (fd == socket->handle()){
            std::cout << str;
        }else{
            socket->write( str );
            if (quit)
                usleep( 100 );
        }
    }
}

int main(int argc, char** argv) {
    chat();

    return 0;
}