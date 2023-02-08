#include "utils.hpp"

namespace SimpicClientLib
{
    /* Get the home folder on UNIX systems. */
    std::string home_folder()
    {
        std::string result = "";

        /* If the effective or the actual user is root, consider them to be the root user calling upon this process, so use the root directory.*/

        if (!getuid() || !geteuid())
        {
            result += "/root/";
            return result;
        }

        char *username = getenv("USER");

        if (username == nullptr)
            return "";

        result += "/home/";
        result += username;
        result += "/";
        
        return result;
    }

    std::string simpic_folder(std::string &home)
    {
        std::string result = "";
        result += home;
        result += ".simpic/";

        return result;
    }

    std::string concatenate_folder(std::string &one, std::string &two)
    {
        std::string result = "";

        result += one;
        result += "/";
        result += two;
        result += "/";

        return result;
    }

    bool good_directory(std::string &where, bool print)
    {
        /* <filesystem> is not really fun... <dirent.h> is way better, albeit less crossplatform. */

        DIR *dir = opendir(where.c_str());

        if (dir == nullptr)
        {
            std::cerr << "Error with directory '" << where << "': " << std::strerror(errno) << std::endl;
            return false;
        }

        closedir(dir);
        return true;
    }

    bool good_path(std::string &where, bool print)
    {
        std::FILE *fp = fopen(where.c_str(), "rb");
        
        if (fp == nullptr)
        {
            if (print)
                std::cout << "Error with path '" << where << "': " << std::strerror(errno) << std::endl;
                
            return false;
        }

        fclose(fp);
        return true;
    }

    void mkdir_dir(std::string &where)
    {
        if (good_directory(where, false))
            return;

        mkdir(where.c_str(), 0755);
    }

    std::string random_chars(uint8_t amount)
    {
        std::string result;
        char bank[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
        
        for (int i = 0; i < amount; i++)
            result += bank[rand() % sizeof(bank)];
        
        return result;
    }

    std::string sha256digest2string(char *digest)
    {
        std::stringstream stream;

        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            stream << std::uppercase << std::hex << digest[i];
        }

        return stream.str();
    }
}