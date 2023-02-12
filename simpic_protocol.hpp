#pragma once

#include <cstdint>
#include <openssl/sha.h>

namespace SimpicClientLib
{
    /* The header which is sent first and only once on each query. It describes how many manual checks that the user is going to have to make, among other things.*/

    enum class MainHeaderCodes
    {
        Success = 0,
        Failure = 1,
        DirectoryAlreadyActive = 2,
        NoResults = 3,
        UnreasonablyLongPath = 4, 
        UnreasonablyLongMaxHam = 5
    };

    enum class DataTypes
    {
        Image = (1),
        Video = (1 << 1),
        Audio = (1 << 2),
        Text = (1 << 3),
        Unspecified = (1 << 4)
    };

    /* The server's reply to the initial client request. */
    struct __attribute__((__packed__)) MainHeader
    {
        uint8_t code; 
        uint8_t _errno;
        uint16_t set_no; 
    };

    /* In this set of similar media types to keep, what type are they and how many are there of them, so that the client can process all of this? */
    struct __attribute__((__packed__)) SetHeader
    {
        uint8_t type; // that of a value in DataTypes.
        uint8_t count; 
    };

    /* This is every image. It describes the file extension, the filename, the width, height, and its length in bytes, all of which are very useful to the client. After this header, it will send the picture data in bytes, the length of which being described by length. */
    struct __attribute__((__packed__)) ImageHeader
    {
        // Not null terminated
        char sha256_hash[SHA256_DIGEST_LENGTH];

        uint16_t width;
        uint16_t height;

        /* Image dimensions and file size. */
        uint32_t size;
        uint16_t filename_length;
        uint16_t path_length; 
        // ^^ if this is -1, then there is no path going to be sent (i.e., recursive mode is off)

        /* Client sends a plea, determining whether they want the filename, the path, dimensions, or the file data. Since they may already have it. */

        // IN THIS ORDER, unless it has been disabled by the plea.
        // read/send filename_length bytes for the null-terminated filename
        // read/send path_length bytes for the null-terminated path: not sent regardless if path_length is -1, even if the client makes a plea.

        // read/send size bytes for the whole file data. 
    };

    enum class ClientRequests
    {
        Exit, // Close the connection, no more requests. 
        Scan, // Scan a directory for similar images. 
        ScanRecursive, // Scan recursively in a directory for similar images. 
        Check, // Check if a file or a set of files would be duplicates in a directory.
        CheckRecursive // Check recursively the same thing as above ^^^
    };

    struct __attribute__((__packed__)) ClientRequest
    {
        uint8_t request;
        uint8_t types; // bitwise field for the file types the client wants.
        uint8_t max_ham; // maximum hamming distance that the client is willing to take. 
        uint16_t path_length;

        // client will send a null-terminated path length. 
    };

    /* A plea containing bitwise flags (abstracted through bitfields) of what the client does not want from the file or whether they want to skip the file entirely. */
    struct __attribute__((__packed__)) ClientPlea
    {
        bool no_data;
        bool skip_file;
    };

    enum class ClientMainPleas
    {
        Continue,
        Stop
    };

    /* A plea for each set of media. This allows the server and client to wait while the client is probably processing the set of images it has. It also allows the client to stop the scan prematurely. */
    struct __attribute__((__packed__)) ClientMainPlea
    {
        uint8_t plea;
    };

    enum class ClientActions
    {
        Keep, // Keep all files. If so, no hashes for deletion will be sent.
        Delete // Delete selected files by their SHA256 hash. 
    };

    struct __attribute__((__packed__)) ClientAction
    {
        uint8_t action;
        uint8_t deletions;
        // an array of indices will then be sent specifying which files should be deleted.
        // the indices should correspond to the order that the files were sent in, 0 indexed.
    };
}