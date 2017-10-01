#include "MySocket.h"
#include <unistd.h>
#include <poll.h>

SocketBase::~SocketBase() {
    std::cout << "~SocketBase\n";
    close();
}

void SocketBase::close(int& fd){
    tr("SocketBase::close ", fd);
    if (fd != -1) {
        shutdown( fd, SHUT_WR );
        shutdown( fd, SHUT_RD );
        if (::close(fd))
            error("Error while close socket");
        fd = -1;
    }
}


std::string SocketBase::read(int fd){
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

void SocketBase::write(int fd, const std::string& str){
    const auto buffer = str.c_str();
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

//////////// Server
bool Server::init(int portno){
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

