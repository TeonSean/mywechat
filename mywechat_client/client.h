#ifndef CLIENT_H
#define CLIENT_H

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>

class Client
{
private:
    int client;

public:
    Client();
    ~Client();

    int tryConnect(const char* ip, int port);
    void closeConnect();
};

#endif // CLIENT_H
