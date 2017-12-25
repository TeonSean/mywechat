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
#include "messagedef.h"
#include <iostream>
#include <set>

#define PORT 8088

class Server
{
private:
    static Server* instance;
    int server;
    std::list<int> clients;
    std::map<int, pthread_t> threads;
    std::map<int, const char*> clientIPs;
    std::map<int, std::string> usernames;
    std::map<std::string, int> usersockets;
    std::map<std::string, std::string> passwords;
    std::map<std::string, std::set<std::string> > friends;
    Server();

public:
    ~Server();
    void loop();
    static void* service_thread(void* p);
    static Server* getInstance();
    static void onConnectionClosed(int fd);
    static void processLogin(int fd);
    static void processLogout(int fd);
    static void processSearch(int fd);
    static void processAdd(int fd);
    static void processList(int fd);
    static void processProfile(int fd);
};

#endif // SERVER_H
