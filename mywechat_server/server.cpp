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

void* Server::service_thread(void *p)
{
    int fd = *(int*)p;
    char smallbuf[128];
    while(true)
    {
        if(recv(fd, smallbuf, 128, 0) <= 0)
        {
            printf("Connection with %s is closed.\n", instance->clientIPs[fd]);
            pthread_exit(NULL);
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
