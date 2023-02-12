# simpic_client
A library and program for interacting with a simpic server (or the current computer), in C++. 

Dependencies:
 - libsimpicserver, can be found [here](https://github.com/emilarner/simpic-server). 
 -  pHash (impossible to compile for Windows, hard to compile for Mac OS)
 - OpenSSL: *libcrypto* and *libssl* 
 - libjpeg
 - libffmpeg
 - libtiff
 - libpng

The topic of most interest to most people is probably the actual client that comes out of this repository, not the details of its library. The client connects to a Simpic server and provides a CLI way to handle/print out similar media files that the server itself detects. Here's the help menu from simpic_client:

    simpic_client - Interfaces with simpic_server to find and deal with duplicate pieces of media.
    USAGE:
    
    -i, --image                        Scan similar images in the search.
               ^~~~~ if not specified, you will be prompted whether to do image only or not.
    -a, --audio                        Scan similar audio in the search.
    -t, --text                         Scan similar text files in the search.
    -v, --video                        Scan similar video files in the search.
    -h, --host [IP/DOMAIN]             The address (domain or IP) of the server.
               ^~~~~ if not specified, the scan will be preformed on the local machine.
    -p, --port [PORT]                  The port number of where the server is on.
    -d, --directory [DIRECTORY]        What directory to scan.
    -r, --recursive                    If on, recursively scan starting from the directory.
    -sd, --send-data [PATH]            If on, download media by hash.
    -n, --no-action                    Don't ask what to keep, just print out similar files.
    -mx, --max-hamming [HAM]           Specify a maximum hamming distance (Default: 3).
    -?, --help                         Shows this menu.

The library allows one to interact with an existing instance of the Simpic server, allowing requests to search directories for similar files and then being able to retrieve the results (and the image data, if desired). The library is modeled in a more asynchronous fashion, as to be more friendly to GUI usages of it. *qtsimpicclient* uses this library to preform its operations, in a nice and pretty GUI way, found [here](https://github.com/emilarner/qtsimpicclient).

For instance, one must provide a callback that gets called every time an image/media file is detected by the Simpic server. Null pointers being sent to the callback indicate the start and the end of a set of images/media files. Even though the library is not written in a particularly C-friendly way (it has absolutely no support for being used by a C program), it does use void pointers to implement generics (we're ultimately C programmers, at the end of the day). You must cast the void pointer to a pointer to the actual datatype that it represents, the type of which is told by an enumerated value passed into the callback as well. We're aware that this is dodgy and that `std::any` would be a much more viable substitute, but at this point, we're not changing it. Honestly, just take a look at the header files if you want to use this library... it's not very nice looking at the moment, but it works. 