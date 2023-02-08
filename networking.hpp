#pragma once

#include <exception>
#include <string>

#include <cstring>
#include <cerrno>


#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

namespace SimpicClientLib
{
    class simpic_networking_exception : std::exception
    {
    private:
        std::string message;

    public:
        uint8_t errnum;

        simpic_networking_exception(std::string msg, uint8_t err);
        std::string &what();
    };

    void recvall(int fd, void *buffer, int length);
    void sendall(int fd, void *buffer, int length);
}