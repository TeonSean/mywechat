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
#include <stack>

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
    std::map<std::string, std::string> chattingWith;
    std::map<std::string, std::list<unsent*> > unsent_msgs;
    std::map<std::string, std::list<file_info*> > unsent_files;
    pthread_mutex_t unsent_lock, chatting_lock;
    Server();

public:
    ~Server();
    void loop();
    static void* service_thread(void* p);
    static Server* getInstance();
    void onConnectionClosed(int fd);
    void processLogin(int fd);
    void processLogout(int fd);
    void processSearch(int fd);
    void processAdd(int fd);
    void processList(int fd);
    void processProfile(int fd);
    void processChat(int fd);
    void processExit(int fd);
    void processSendMsg(int fd);
    void processRecvMsg(int fd);
    void processSendFile(int fd);
    void processRecvFile(int fd);
    void sendAction(int fd, char action);
    void sendName(int fd, std::string name);
    void sendFileName(int fd, std::string name);
    void sendInt(int fd, int n);
    void sendMessage(int fd, std::string msg);
    std::string receiveMessage(int fd);
    int receiveInt(int fd);
    std::string receiveName(int fd);
    std::string receiveFileName(int fd);
    char receiveAction(int fd);
    void receiveFile(int fd, char* buf, int size);
    void sendFile(int fd, char* buf, int size);
};

#endif // SERVER_H
