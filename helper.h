#ifndef CHAT_HELPER_H
#define CHAT_HELPER_H

#include <iostream>

inline void tr( const char* msg){
    std::cout << msg << std::endl;
}

inline void tr( const char* msg, int i){
    std::cout << msg << i << std::endl;
}

inline void error(const char *msg, ...){
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);

    perror("\n");
    exit(1);
}


#endif //CHAT_HELPER_H
