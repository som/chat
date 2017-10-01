#include <assert.h>
#include "PImpl.h"
#include "MySocket.h"
#include "inetserverstream.hpp"
#include "select.hpp"


class PImpl::Socket{
    libsocket::inet_stream_server _libsrv;
    std::unique_ptr<libsocket::inet_stream> _libclient;
    std::unique_ptr<SocketBase> _my;
    const int PORT = 8080;
    const char* HOST = "localhost";

    friend class PImpl;
    const std::string read(){
        if (_libclient){
            string res;
            res.resize( 100 );
            (*_libclient) >> res;
            return res;
        }

        return _my->read();
    }

    void write(const std::string& str){
        if (_libclient)
            (*_libclient) << str;
        else
            _my->write( str );
    }

    int handle() const{
        if (_libclient)
            return _libclient->getfd();

        return _my->handle();
    }


    bool makeMyClient(){
        assert( !_libclient && !_my );

        _my.reset( new Client() );
        if (dynamic_cast<Client*>( _my.get() )->init(HOST, PORT)) return true;

        _my.reset( nullptr );
        return false;
    }

    bool makeMyServer(){
        assert( !_libclient && !_my );

        _my.reset( new Server() );
        if (dynamic_cast<Server*>( _my.get() )->init(PORT)) return true;

        _my.reset( nullptr );
        return false;
    }

    bool makeLibClient(){
        assert( !_libclient && !_my );

        try {
            std::unique_ptr<libsocket::inet_stream> libclient( new libsocket::inet_stream() );
            libclient->connect(HOST, std::to_string(PORT).c_str(), LIBSOCKET_IPv4);
            _libclient.reset( libclient.release() );
            return true;
        }
        catch(...){
            std::cout << "libsocket client fail" << std::endl;
        }

        return false;
    }

    bool makeLibServer(){
        assert( !_libclient && !_my );

        _libsrv.setup(HOST, std::to_string(PORT).c_str(), LIBSOCKET_IPv4);
        _libclient = _libsrv.accept2();
        return true;
    }
};

std::unique_ptr<PImpl> PImpl::make( char type ){
    std::unique_ptr<PImpl> res( new PImpl() );
    if (type == 'm'){
        res->_p->makeMyClient() || res->_p->makeMyServer();
    }else{
        res->_p->makeLibClient() || res->_p->makeLibServer();
    }

    return res;
}

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

PImpl::PImpl():_p( new Socket() ){
}

PImpl::~PImpl(){
    reset();
}

void PImpl::reset(){
    if (_p != nullptr){
        delete _p;
        _p = nullptr;
    }
}

void PImpl::write(const std::string& str){
    _p->write( str );
}

std::string PImpl::read(){
    return _p->read();
}

int PImpl::waitAndRead( int additionalHandle, std::string& str ){
    int afd[] = { additionalHandle, _p->handle() };
    int fd = findHandleForRead( afd );

    if (fd == _p->handle())
        str = _p->read();

    return fd;
}