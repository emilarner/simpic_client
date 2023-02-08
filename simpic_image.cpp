#include "simpic_image.hpp"

namespace SimpicClientLib
{
    Image::Image(struct ImageHeader *hdr, int _index, int _fd)
    {
        currently_read = 0;

        index = _index;
        width = hdr->width;
        height = hdr->height;
        fd = _fd;
        std::memcpy(sha256, hdr->sha256_hash, sizeof(sha256));

        length = hdr->size;

        /* Initialize VLAs for the filename and path information. */
        char c_filename[hdr->filename_length] = {0};
        char c_pathname[hdr->path_length] = {0};

        recvall(fd, c_filename, sizeof(c_filename));
        recvall(fd, c_pathname, sizeof(c_pathname));

        std::cout << c_filename << std::endl;
        std::cout << c_pathname << std::endl;

        filename += c_filename;
        path += c_pathname;
    }

    size_t Image::readbytes(char *buf, size_t amnt)
    {
        /* The entire image was read... we don't need to read from it anymore. */
        if (currently_read == length)
            return -1;

        /* If the next read would exceed the length of the file. */
        if ((currently_read + amnt) > length)
        {
            amnt = length - currently_read;
        }

        size_t amntread = recv(fd, buf, amnt, MSG_WAITALL);
        currently_read += amntread;

        return amntread;
    }
}