#ifndef CHAT_PIMPL_H
#define CHAT_PIMPL_H

#include <memory>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class PImpl{
protected:
    class Socket;
    Socket* _p;

    PImpl();
    PImpl(const PImpl&) = delete;

public:
    ~PImpl();

    void reset();
    static std::unique_ptr<PImpl> make( char type );
    std::string read();
    void write(const std::string& str);
    int handle() const;
    int waitAndRead( int additionalHandle, std::string& str );
};

#endif //CHAT_PIMPL_H
