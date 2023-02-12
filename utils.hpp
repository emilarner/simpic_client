#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include <unistd.h>
#include <dirent.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <sys/stat.h>
#include <openssl/sha.h>

#include <ctime>

namespace SimpicClientLib
{
    std::string home_folder();
    std::string simpic_folder(const std::string &home);
    std::string concatenate_folder(std::string &one, std::string &two);

    bool good_directory(std::string &where, bool print);
    bool good_path(std::string &where, bool print);

    /* Create directory if it does not exist. */
    void mkdir_dir(std::string &where);
    std::string random_chars(uint8_t amount);


    std::string sha256digest2string(char *digest);
}