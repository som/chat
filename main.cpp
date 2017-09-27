
#include "libsocket.h"
#include "MySocket.h"
#include <iostream>

template < int size >
static int findHandleForRead(const int (&afd)[ size ]){
    poll::pollfd apollfd[size];
    memset(apollfd, 0, sizeof(apollfd));
    timeval tv = { .tv_sec = 0, .tv_usec = 100 };

    for(int i=0; i < size; i++){
        apollfd[i].fd = afd[i];
        apollfd[i].events = POLLIN;
    }

    int retval = poll::poll(apollfd, size, -1);
    if (retval > 0) {
        for (int i = 0; i < size; i++) {
            if (apollfd[i].revents & POLLIN)
                return apollfd[i].fd;
        }
    }

    return -1;
}

std::unique_ptr<SocketBase> findSocket() throw(){
    static const int PORT = 8080;

    std::unique_ptr<SocketBase> res( new Client() );
    if (static_cast<Client*>( res.get() )->init("localhost", PORT)) return res;

    res.reset( new Server() );
    if (static_cast<Server*>( res.get() )->init( PORT ) == false){
        error("Can't open server");
    }

    return res;
}

void chat(){
    std::unique_ptr<SocketBase> socket( findSocket() );
    std::cout << "Start chat:\n";

    int afd[] = { ::stdin->_file, socket->handle() };
    for(bool quit = false; !quit ;){
        int fd = findHandleForRead( afd );
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
//    if (argc == 2) {
//        std::cout << "arg:" << argv[1] << std::endl;
////        if (*argv[1] == 's')
////        else
//    }

    return 0;
}