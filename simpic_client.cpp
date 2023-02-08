#include "simpic_client.hpp"

namespace SimpicClientLib
{
    NoResultsException::NoResultsException(std::string msg)
    {
        message = msg;
    }

    std::string &NoResultsException::what()
    {
        return message;
    }

    LimitsException::LimitsException(std::string msg, std::string thng)
    {
        message = msg;
        thing = thng;
    }

    std::string &LimitsException::what()
    {
        return message;
    }

    InUseException::InUseException(std::string msg, std::string _folder)
    {
        message = msg;
        folder = _folder;
    }

    std::string &InUseException::what()
    {
        return message;
    }

    ErrnoException::ErrnoException(int errnum)
    {
        message = std::string(std::strerror(errnum));
        _errno = errnum;
    }

    std::string &ErrnoException::what()
    {
        return message;
    }

    SimpicClient::SimpicClient(std::string &addr, uint16_t port)
    {
        no_data = false;

        struct hostent *entry = gethostbyname(addr.c_str());

        /* If the address/domain resolution failed, complain heavily about it. */
        if (entry == nullptr)
            throw std::runtime_error("Could not resolve address: " + std::string(hstrerror(h_errno)));

        server_addr = *((struct in_addr*) entry->h_addr_list[0]);
        saddr.sin_addr = server_addr;
        saddr.sin_port = htons(port);
        saddr.sin_family = AF_INET;

        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        connected = false;
    }

    int SimpicClient::make_connection()
    {
        /* Try to connect--if it failed, returned the errno value for the error. */
        if (connect(fd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0)
            throw simpic_networking_exception("connect() failed in SimpicClient", errno);

        connected = true;
        return 0; 
    }

    void SimpicClient::remove(std::vector<int> &selected)
    {
        struct ClientAction act;
        act.action = (uint8_t) ClientActions::Delete;
        act.deletions = selected.size();
        sendall(fd, &act, sizeof(act));

        /* Send the indices which are to be deleted. */
        for (int index : selected)
        {
            sendall(fd, (uint8_t*) &index, sizeof(uint8_t));
        }
    }

    void SimpicClient::keep()
    {
        struct ClientAction act;
        act.deletions = -1;
        act.action = (uint8_t) ClientActions::Keep;

        sendall(fd, &act, sizeof(act));
    }

    int SimpicClient::request(std::string &path, bool recursive, uint8_t max_ham, uint8_t types,
                        std::function<void(void*, DataTypes)> callback)
    {
        /* Send a structure that tells the server what we want. */
        struct ClientRequest req;
        req.max_ham = max_ham;
        req.path_length = path.size() + 1;
        req.types = types;
        req.request = (uint8_t)(recursive ? ClientRequests::ScanRecursive : ClientRequests::Scan);


        sendall(fd, &req, sizeof(req));
        sendall(fd, (char*)path.c_str(), req.path_length);

        /* The server is now going to tell us how many results it found. */
        struct MainHeader mhdr;
        recvall(fd, &mhdr, sizeof(mhdr));

        if (mhdr.code == (uint8_t)MainHeaderCodes::NoResults)
        {
            throw NoResultsException("Simpic server found no similar images.");
            return -1;
        }

        /* If the server complains of the directory already being scanned. */
        if (mhdr.code == (uint8_t)MainHeaderCodes::DirectoryAlreadyActive)
        {
            throw InUseException(
                "A scan cannot be preformed--the directory is already being scanned.",
                path
            );

            return -1;
        }

        if (mhdr.code == (uint8_t)MainHeaderCodes::Limits)
        {
            char limits_msg[8];
            memset(limits_msg, 0, sizeof(limits_msg));
            recvall(fd, limits_msg, sizeof(limits_msg));

            throw LimitsException(
                "The client--or you--sent something that was too long or incorrect for the server.",
                std::string(limits_msg)
            );

            return -1;
        }

        /* A generic--we don't know exactly--error occured, let's see its errno. */
        if (mhdr.code == (uint8_t)MainHeaderCodes::Failure)
        {
            throw ErrnoException(mhdr._errno);
            return mhdr._errno;
        }

        /* Memory garbage can. */
        std::vector<void*> garbage;

        /* Loop the amount of times we expect a set.*/
        for (int i = 0; i < mhdr.set_no; i++)
        {
            /* Empty and free the garbage can. */
            for (void *ptr : garbage)
                delete ptr; 

            garbage.clear();

            struct SetHeader shdr;
            recvall(fd, &shdr, sizeof(shdr));

            switch ((DataTypes)shdr.type)
            {
                /* If the set is a set of images. */
                case DataTypes::Image:
                {
                    /* This signifies the start of a collection of images. */
                    callback(nullptr, DataTypes::Image);

                    /* Let the client handle every image that comes through. */
                    for (int j = 0; j < shdr.count; j++)
                    {
                        struct ImageHeader ihdr;
                        recvall(fd, &ihdr, sizeof(ihdr));

                        Image *img = new Image(&ihdr, j, fd);
                        garbage.push_back(img);

                        /* The server needs to know whether to send the file data. */
                        struct ClientPlea plea;
                        plea.no_data = no_data;
                        plea.skip_file = false;
                        sendall(fd, &plea, sizeof(plea));

                        /* void* > std::any */
                        /* BLOCKS this thread. */
                        callback((void*) img, DataTypes::Image);
                    }

                    /* This signals that the end of the set has been reached. */
                    callback(nullptr, DataTypes::Image);
                    break;
                }

                default:
                {
                    break;
                }
            }
        }

        return 0;    
    }

    void SimpicClient::handler()
    {
        while (true)
        {

        }
    }

    void SimpicClient::set_no_data(bool data)
    {
        no_data = data;
    }

    void SimpicClient::close()
    {
        struct ClientRequest req;
        req.request = (uint8_t) ClientRequests::Exit;
        sendall(fd, &req, sizeof(req));
    }
}