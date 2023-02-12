#pragma once

#include <iostream>
#include <vector>
#include <openssl/ssl.h>

#include <cstring>

#include "simpic_protocol.hpp"
#include "networking.hpp"

namespace SimpicClientLib
{
    enum class ImageType
    {
        PNG,
        JPEG,
        GIF,
        Undefined
    };

    class Image
    {
    private:
        int fd;
        size_t currently_read;
    public:
        char sha256[SHA256_DIGEST_LENGTH];
        size_t length;

        int index;
        int no_sets;
        int set_no;

        uint16_t width;
        uint16_t height;
        
        std::string filename;
        std::string path; 
        
        ImageType type; 

        Image(struct ImageHeader *hdr, int _index, int _fd);

        /* If read mode was turned on, read until this returns -1. */
        size_t readbytes(char *buf, size_t amnt);
    };
}