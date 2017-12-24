#include"server.h"

Server* Server::instance = NULL;

Server::Server()
{
    server = socket(AF_INET, SOCK_STREAM, 0);
    if(server == -1)
    {
        perror("Failed to create socket.");
        exit(-1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(server, (sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("bind error");
        exit(-1);
    }
    if(listen(server, 100) == -1)
    {
        perror("listen error");
        exit(-1);
    }
    struct sockaddr_in sin;
    struct ifreq ifr;
    strncpy(ifr.ifr_name, "wlp8s0", IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    if(ioctl(server, SIOCGIFADDR, &ifr) < 0)
    {
        perror("ioctl error");
        close(server);
        exit(-1);
    }
    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    printf("Server started.\n\n");
    printf("Listening on %s, port %d.\n\n", inet_ntoa(sin.sin_addr), PORT);
    printf("Waiting for incoming connection...\n\n");
}

void Server::processLogout(int fd)
{
    char re = (char)SUCCEESS;
    send(fd, &re, 1, 0);
    instance->usernames.erase(fd);
}

void Server::processLogin(int fd)
{
    char* buf = new char[sizeof(login_packet)];
    if(recv(fd, buf, sizeof(login_packet), 0) <= 0)
    {
        printf("Connection with %s is closed.\n\n", instance->clientIPs[fd]);
        pthread_exit(NULL);
    }
    login_packet* lp = (login_packet*)buf;
    std::string usrname, psword;
    usrname =usrname.assign(lp->name, (int)lp->namelen);
    psword = psword.assign(lp->code, (int)lp->codelen);
    char re;
    if(instance->usersockets.count(usrname))
    {
        re = (char)ALREADY_ONLINE;
        send(fd, &re, 1, 0);
        printf("Username %s already online. Logging denied.\n\n", usrname.c_str());
        instance->usernames.erase(fd);
        return;
    }
    if(instance->passwords.count(usrname))
    {
        if(psword != instance->passwords[usrname])
        {
            re = (char)WRONG_PASSWORD;
            send(fd, &re, 1, 0);
            printf("Username %s and password %s don't match.\n\n", usrname.c_str(), psword.c_str());
            instance->usernames.erase(fd);
            instance->usersockets.erase(usrname);
            return;
        }
        re = (char)SUCCEESS;
        send(fd, &re, 1, 0);
        printf("IP %s logged in as user %s.\n\n", instance->clientIPs[fd], usrname.c_str());
        instance->usernames[fd] = usrname;
        instance->usersockets[usrname] = fd;
    }
    else
    {
        re = (char)ACCOUNT_CREATED;
        send(fd, &re, 1, 0);
        printf("IP %s created account. Username: %s, password: %s.\n\n", instance->clientIPs[fd], usrname.c_str(), psword.c_str());
        instance->usernames[fd] = usrname;
        instance->passwords[usrname] = psword;
        instance->usersockets[usrname] = fd;
    }
}

void* Server::service_thread(void *p)
{
    int fd = *(int*)p;
    char action;
    while(true)
    {
        if(recv(fd, &action, 1, 0) <= 0)
        {
            printf("Connection with %s is closed.\n\n", instance->clientIPs[fd]);
            pthread_exit(NULL);
        }
        switch(action)
        {
        case ACTION_LOGIN:
            printf("IP %s requested logging in.\n\n", instance->clientIPs[fd]);
            processLogin(fd);
            break;
        case ACTION_LOGOUT:
            printf("IP %s requested logging out from %s.\n\n", instance->clientIPs[fd], instance->usernames[fd].c_str());
            processLogout(fd);
            break;
        default:
            printf("Invalid actioin: %d.\n\n", (int)action);
        }
    }
}

Server* Server::getInstance()
{
    if(instance == NULL)
    {
        instance = new Server();
    }
    return instance;
}

void Server::loop()
{
    while(true)
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int fd = accept(server, (sockaddr*)&addr, &len);
        if(fd == -1)
        {
            printf("Client encounters an error.\n\n");
            continue;
        }
        sockaddr_in info;
        socklen_t infolen = sizeof(info);
        if(getpeername(fd, (sockaddr*)&info, &infolen) == -1)
        {
            printf("Failed to get information of client.\n\n");
            continue;
        }
        clients.push_back(fd);
        clientIPs[fd] = inet_ntoa(info.sin_addr);
        printf("Connection from %s accepted.\n\n", clientIPs[fd]);
        threads[fd] = pthread_t();
        pthread_create(&threads[fd], NULL, service_thread, &clients.back());
    }
}

Server::~Server()
{
    printf("Server shut down.\n\n");
    close(server);
}
