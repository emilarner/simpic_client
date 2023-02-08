#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

#include <cstring>
#include <cstdlib>

#include <simpic_server/simpic_server.hpp>

#include "utils.hpp"
#include "config.h"
#include "simpic_client.hpp"

using namespace SimpicClientLib;

enum class Modes
{
    Images = 1,
    Video = 1 << 1,
    Audio = 1 << 2,
    Text = 1 << 3,
    Recursive = 1 << 4,
    NoData = 1 << 5
};

void help()
{
    const char *help_text =
    "simpic_client - Interfaces with simpic_server to find and deal with duplicate pieces of media.\n"
    "USAGE:\n\n"
    "-i, --image                        Scan similar images in the search.\n"
    "           ^~~~~ if not specified, you will be prompted whether to do image only or not.\n"
    "-a, --audio                        Scan similar audio in the search.\n"
    "-t, --text                         Scan similar text files in the search.\n"
    "-v, --video                        Scan similar video files in the search.\n"
    "-h, --host [IP/DOMAIN]             The address (domain or IP) of the server.\n"
    "           ^~~~~ if not specified, the scan will be preformed on the local machine.\n"
    "-p, --port [PORT]                  The port number of where the server is on.\n"
    "-d, --directory [DIRECTORY]        What directory to scan.\n"
    "-r, --recursive                    If on, recursively scan starting from the directory.\n"
    "-sd, --send-data [PATH]            If on, download media by hash.\n"
    "-mx, --max-hamming [HAM]           Specify a maximum hamming distance (Default: 3).\n"
    "-?, --help                         Shows this menu.\n\n";

    std::cout << help_text << std::endl;
}

int main(int argc, char **argv, char **envp)
{
    bool local = false;

    std::string homedir = home_folder();
    std::string ourfolder = simpic_folder(homedir);
    std::string recycling_bin = ourfolder + "recycling_bin/";

    std::thread *localserver = nullptr;

    uint16_t port = 0;
    const char *directory = nullptr;
    const char *address = nullptr;

    uint8_t mode = 0;

    /* Argument parsing. */
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
            continue;

        if (!std::strcmp(argv[i], "-i") || !std::strcmp(argv[i], "--images"))
            mode |= (uint8_t) Modes::Images;

        else if (!std::strcmp(argv[i], "-v") || !std::strcmp(argv[i], "--videos"))
            mode |= (uint8_t) Modes::Video;

        else if (!std::strcmp(argv[i], "-a") || !std::strcmp(argv[i], "--audio"))
            mode |= (uint8_t) Modes::Audio;

        else if (!std::strcmp(argv[i], "-t") || !std::strcmp(argv[i], "--text"))
            mode |= (uint8_t) Modes::Text;

        else if (!std::strcmp(argv[i], "-r") || !std::strcmp(argv[i], "--recursive"))
            mode |= (uint8_t) Modes::Recursive;

        else if (!std::strcmp(argv[i], "-?") || !std::strcmp(argv[i], "--help"))
        {
            help();
            return 0;
        }

        else if (!std::strcmp(argv[i], "-d") || !std::strcmp(argv[i], "--directory"))
        {
            /* If the argument is the end of the nullptr-terminated string array. */
            if (argv[i + 1] == nullptr)
            {
                std::cerr << "-d/--directory requires an argument (the directory to scan). \n";
                return -1;
            }

            directory = argv[i + 1];
        }

        else if (!std::strcmp(argv[i], "-h") || !std::strcmp(argv[i], "--host"))
        {
            if (argv[i + 1] == nullptr)
            {
                std::cerr << "-h/--host requires a host (the server's IP).\n";
                return -1;
            }

            address = argv[i + 1];
        }

        else if (!std::strcmp(argv[i], "-p") || !std::strcmp(argv[i], "--port"))
        {
            if (argv[i + 1] == nullptr)
            {
                std::cerr << "-p/--port requires a port (the server's port).\n";
                return -1;
            }

            /* Attempt to convert the port from a string back into an integer value.*/
            try
            {
                port = std::stoi(std::string(argv[i + 1]));
            }
            catch (std::out_of_range &ex)
            {
                std::cerr << "Error parsing port: " << ex.what() << std::endl;
                return -1;
            }
        }

        else
        {
            std::cerr << "Unrecognized command-line argument '" << argv[i] << "'.\n";
            return -1;
        }
    }

    /* If no mode was specified. */
    if (!mode)
    {
        std::cerr << "Warning: no mode was specified... Do you want to just do images only? Y/y N/n: ";

        /* Get the next character, convert it to lowercase using a bithack, and see if its NOT 'y'.*/
        if ((char)(std::cin.get() | (1 << 5)) != 'y')
        {
            std::cerr << "\nOkay, bye!\n";
            return -1;
        }

        mode = (uint8_t)Modes::Images;
    }

    /* This program requires a directory. */
    if (directory == nullptr)
    {
        std::cerr << "The simpic_client requires a directory to scan... and it was not provided.\n";
        std::cerr << "Exiting... you can supply one with -d/--directory [DIRNAME] on the remote end.\n";
        return -1;
    }

    std::string cpp_directory(directory);

    /* If no address was specified, start our own local server. */
    if (address == nullptr)
    {
        std::cerr << "Address not specified--searching locally.\n";
        std::thread hosting_server([&ourfolder, &recycling_bin]() -> void {
            SimpicServerLib::SimpicServer srv((uint16_t)MOCK_PORT, ourfolder, recycling_bin);
            srv.start();
        });

        hosting_server.detach();

        /* bad practice, but it just werkz*/
        std::this_thread::sleep_for(std::chrono::seconds(CHEAP_THREAD_SLEEP_TIME));

        local = true;
        address = "0.0.0.0";

    }

    std::string cpp_address(address);

    /* If no port was specified. */
    if (!port && address != nullptr && !local)
    {
        std::cerr << "Either the port was set to 0 (invalid) or a port wasn't given... \n";
        std::cerr << "This is a critical error and thus we must shut down.\n";
        return -1;
    }

    bool in_set = false;
	uint32_t highest_index = 0;

    /* MOCK_PORT if hosting locally. */
    SimpicClient client(cpp_address, local ? MOCK_PORT : port);

    try 
    {
        client.make_connection();
        client.set_no_data(true);

        client.request(cpp_directory, mode & (uint8_t)Modes::Recursive, 3, mode, [&in_set, &highest_index, &client](void *data, DataTypes type) mutable -> void {
            /* Beginning of a set. */
            if (data == nullptr && !in_set)
            {
                std::cout << std::endl;
                in_set = true;
                return;
            }

            /* End of a set. */
            if (data == nullptr && in_set)
            {
    index_parsing:
                std::cout << "Please enter the comma delimited indices of the files to delete. \n";
                std::cout << "Example: 0,2,4,5\n";
                std::cout << "Write nothing to keep all files.\n";

                std::vector<int> indices;

                std::string input;
                std::getline(std::cin, input);

                /* It must be allocated on the heap. */
                char *c_input = new char[input.size() + 1];
                std::strcpy(c_input, input.c_str());

                char *token = std::strtok(c_input, ",");

                /* Keep all files. */
                if (token == nullptr)
                {
                    std::cout << "Files kept." << std::endl; 
                    client.keep();
                    delete[] c_input;
                    in_set = false;
                    return;
                }

                /* Grab files. */
                while (token != nullptr)
                {
                    try
                    {
                        /* Parse indices. */
                        uint32_t our_index = std::stoi(std::string(token));
                        
                        /* Invalid index. */
                        if (our_index > highest_index)
                        {
                            std::cerr << "What in the world are you doing, giving an image index that does not exist?" << std::endl;
                            delete[] c_input;
                            goto index_parsing;
                        }
                        
                        indices.push_back(std::stoi(std::string(token)));
                        token = std::strtok(nullptr, ",");
                    }
                    catch (std::exception &ex)
                    {
                        std::cerr << "You entered the indices wrong because: " << ex.what();
                        std::cerr << "... did you add a space between the entries?\n";
                        delete[] c_input;

                        /* The only acceptable usage of goto--deeply nested loops. */
                        /* This goto asks for the data again, since it was incorrect. */
                        goto index_parsing;
                    }
                }
                
                std::cout << "Deleting these: ";
                std::for_each(indices.begin(), indices.end(), [](int &val) -> void { std::cout << val << " "; });
                std::cout << std::endl;


                delete[] c_input;

                /* Tell the server to remove these. */
                client.remove(indices);
                indices.clear();

                std::cout << "Moves past." << std::endl;

                in_set = false;
                // deprecated
                //client.next_set();
                return;
            }

            /* Data about a file was sent through. */
            switch (type)
            {
                case DataTypes::Image:
                {
                    Image *img = (Image*) data;

                    /* Update the store of the highest index. */
                    if (img->index > highest_index)
                        highest_index = img->index;

                    /*
                    std::cout << "\n\n";
                    std::cout << "Index: " << img->index << "\n";
                    std::cout << "Image: \n";
                    std::cout << "Filename: " << img->filename << "\n";
                    std::cout << "Path: " << img->path << "\n";
                    std::cout << "Dimensions: " << img->width << "x" << img->height << "\n";
                    */

                    std::cout << "[" << img->index << "]: " << img->path << "/" << img->filename << " " << img->width << "x" << img->height << std::endl;

                    break;
                }
            }
        });
    }
    catch (InUseException &ex)
    {
        std::cerr << ex.what() << std::endl;
        std::cerr << "Folder: " << ex.folder << std::endl;
        return -1;
    }
    catch (ErrnoException &ex)
    {
        std::cerr << ex.what() << std::endl;
        std::cerr << "Errno: " << ex._errno << std::endl;
        return -1;
    }
    catch (simpic_networking_exception &ex)
    {
        std::cerr << "Networking error: " << ex.what() << std::endl;
        std::cerr << "Errno Text: " << std::strerror(ex.errnum) << std::endl;
        return -1;
    }
    catch (std::exception &ex)
    {
        std::cerr << "General unknown exception: "  << ex.what() << std::endl;
        return -1;
    }

    client.close();
    return 0;
}
