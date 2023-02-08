CC=g++
CPPFLAGS=-g -std=c++20
LIBS=-L$(shell pwd) -lsimpicserver -lpHash -ljpeg -ltiff -lpng -lssl -lcrypto

simpic_client: libsimpicclient.so main.o
	$(CC) $(CPPFLAGS) -o simpic_client main.o -lsimpicclient $(LIBS)

libsimpicclient.so: simpic_client.o networking.o utils.o simpic_image.o simpic_protocol.hpp utils.o
	$(CC) $(CPPFLAGS) -shared -o libsimpicclient.so simpic_client.o networking.o utils.o simpic_image.o

simpic_client.o: simpic_client.cpp
	$(CC) $(CPPFLAGS) -fPIC -c simpic_client.cpp

networking.o: networking.cpp
	$(CC) $(CPPFLAGS) -fPIC -c networking.cpp

utils.o: utils.cpp
	$(CC) $(CPPFLAGS) -fPIC -c utils.cpp

simpic_image.o: simpic_image.cpp
	$(CC) $(CPPFLAGS) -fPIC -c simpic_image.cpp

main.o: main.cpp config.h
	$(CC) $(CPPFLAGS) -c main.cpp

install:
	cp libsimpicclient.so /usr/lib/
	mkdir -p /usr/include/simpic_client/
	cp *.hpp /usr/include/simpic_client/
	cp simpic_client /usr/bin/

	chmod 0755 /usr/lib/libsimpicclient.so
	chmod -R 0755 /usr/include/simpic_client/
	chmod 0755 /usr/bin/simpic_client

clean:
	rm *.so
	rm *.o
	rm simpic_client