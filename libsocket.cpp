#include "libsocket.h"

# include <iostream>
# include <string>
# include <unistd.h>
# include <stdio.h>
# include <utility>
# include <memory>
#include "helper.h"

//// LSServer
 bool LSServer::init(int portno){
    _srv.setup("localhost", std::to_string(portno).c_str(), LIBSOCKET_IPv6);
    _client = _srv.accept2();
    return true;
}

std::string LSServer::read(){
    string s;
    *_client >> s;
    return s;
}

void LSServer::write(const std::string& str){
    *_client << str;
}

//// LSClient
bool LSClient::init(const char* servername, int portno){
    try {
        _client.connect(servername, std::to_string(portno).c_str(), LIBSOCKET_IPv6);
        return true;
    }
    catch(...){
        std::cout << "libsocket client fail" << std::endl;
    }
    return false;
}

std::string LSClient::read(){
    string s;
    _client >> s;
    return s;
}

void LSClient::write(const std::string& str){
    _client << str;
}


/////////
std::unique_ptr<ISocketBase> makeLibSocket() throw(){
    static const int PORT = 8080;

    std::unique_ptr<ISocketBase> res( new LSClient() );
    if (static_cast<LSClient*>( res.get() )->init("localhost", PORT)) return res;

    res.reset( new LSServer() );
    if (static_cast<LSServer*>( res.get() )->init( PORT ) == false){
        error("Can't open server");
    }

    return res;
}
