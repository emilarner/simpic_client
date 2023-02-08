#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <exception>

#include <cstring>
#include <cstdlib>
#include <cerrno>

#include <functional>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>

#include "networking.hpp"
#include "simpic_image.hpp"
#include "simpic_protocol.hpp"
#include "utils.hpp"

namespace SimpicClientLib
{
    typedef DataTypes SimpicClientTypes;

    class NoResultsException : std::exception
    {
    public:
        std::string message;

        NoResultsException(std::string msg);
        std::string &what();
    };

    class LimitsException : std::exception
    {
    public:
        std::string message;
        std::string thing;

        LimitsException(std::string msg, std::string thng);
        std::string &what();
    };

    class InUseException : std::exception
    {
    public:
        std::string message;
        std::string folder;

        InUseException(std::string msg, std::string _folder);
        std::string &what();
    };

    class ErrnoException : std::exception
    {
    public:
        std::string message;
        int _errno;
        
        ErrnoException(int errnum);
        std::string &what();
    };

    class SimpicClient
    {
    private:
        bool locked;
        bool connected;
        bool no_data;

        std::string dfolder;

        void handler();
    public:
        struct in_addr server_addr;
        struct sockaddr_in saddr;
        int fd;
        uint16_t port;

        std::string cache_location;

        /* Initialize a client where addr and port form the address of the server. */
        SimpicClient(std::string &addr, uint16_t port);

        /* Connect to the server if possible. Returns the errno value if it valued; otherwise, it doesn't return, instead calling the handler which blocks the main thread. */
        int make_connection();

        /* After the collections are received, pass a vector of ints to describe which you want to delete. */
        void remove(std::vector<int> &selected);

        /* Keep everything. */
        void keep();


        /* Send the request to the server to scan a path for duplicate media. */
        /* The callback will help you: */
        /* In the beginning of a set, the first argument will be a nullptr. */
        /* Other items of the set will point to a valid object, the type specified by the second parameter*/
        /* When the set is done, another nullptr will be sent. It is up to you to keep track of this. */
        /* Then it shall repeat until it is no longer called. */
        int request(std::string &path, bool recursive, uint8_t max_ham, uint8_t types,
                        std::function<void(void*, DataTypes)> callback);

        
        /* When making a request for similar images, do you want to not the server to send the image itself over? This saves time and bandwidth, especially for very large files. */
        void set_no_data(bool data);

        /* After everything is said and done, exit without a hitch. */
        void close();
    };
}