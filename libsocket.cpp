#include "libsocket.h"

# include <iostream>
# include <string>
# include <unistd.h>
# include <stdio.h>
# include <utility>
# include <memory>

void libChatServer(){
    using libsocket::inet_stream_server;
    using libsocket::inet_stream;
    using libsocket::selectset;
    string answ;

    inet_stream_server srv("localhost","8080",LIBSOCKET_IPv6);
    selectset<inet_stream_server> set1;
    set1.add_fd(srv,LIBSOCKET_READ);

    for ( ;; )
    {
        /********* SELECT PART **********/
        std::cout << "Called select()\n";

        libsocket::selectset<inet_stream_server>::ready_socks readypair; // Create pair (libsocket::fd_struct is the return type of selectset::wait()

        readypair = set1.wait(-1); // Wait for a connection and save the pair to the var
        if (readypair.first.size() == 0){
            continue;
        }


        inet_stream_server* ready_srv = dynamic_cast<inet_stream_server*>(readypair.first.back()); // Get the last fd of the LIBSOCKET_READ vector (.first) of the pair and cast the socket* to inet_stream_server*

        readypair.first.pop_back(); // delete the fd from the pair

        std::cout << "Ready for accepting\n";

        /*******************************/

        std::unique_ptr<inet_stream> cl1 = ready_srv->accept2();

        *cl1 << "Hello\n";

        answ.resize(100);

        *cl1 >> answ;

        std::cout << answ;

        // cl1 is closed automatically when leaving the scope!
    }

    srv.destroy();

}

void libChatClient(){
    using std::string;

    using libsocket::inet_stream;

    string host = "::1";
    string port = "8080";
    string answer;

    answer.resize(32);

    try {
        libsocket::inet_stream sock(host,port,LIBSOCKET_IPv6);

        sock >> answer;

        std::cout << answer;

        sock << "Hello back!\n";

        // sock is closed here automatically!
    } catch (const libsocket::socket_exception& exc)
    {
        std::cerr << exc.mesg;
    }
}


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
    _client.connect(servername, std::to_string(portno).c_str(), LIBSOCKET_IPv6);
    return true;
}

std::string LSClient::read(){
    string s;
    _client >> s;
    return s;
}

void LSClient::write(const std::string& str){
    _client << str;
}
