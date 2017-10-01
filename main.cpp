
#include "PImpl.h"
#include <iostream>
#include <zconf.h>

void chat(char type){
    std::unique_ptr<PImpl> socket( PImpl::make( type ) );
    std::cout << "Start chat:\n";

    std::string str;
    do{
        str.erase();
        int fd = socket->waitAndRead( ::stdin->_file, str );
        if (fd < 0) continue;

        if (fd == ::stdin->_file) {
            std::cin >> str;
            str += "\n";
            socket->write( str );
        }else {
            std::cout << str;
        }
    }while(str != "quit\n");

    usleep( 100 );
}

int main(int argc, char** argv) {
    chat( argc == 2 ? *argv[1] : 0 );

    return 0;
}