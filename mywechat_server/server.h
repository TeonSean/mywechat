#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <pthread.h>
#include <net/if.h>
#include <list>
#include <map>

#define PORT 8088

class Server
{
private:
    static Server* instance;
    int server;
    std::list<int> clients;
    std::map<int, pthread_t> threads;
    std::map<int, const char*> clientIPs;
    Server();

public:
    ~Server();
    void loop();
    static void* service_thread(void* p);
    static Server* getInstance();
};

#endif // SERVER_H
