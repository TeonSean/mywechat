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
    std::cout << "Server started.\n\n";
    std::cout << "Listening on " << inet_ntoa(sin.sin_addr) << ", port " << PORT << ".\n\n";
    std::cout << "Waiting for incoming connection...\n\n";
}

void Server::processLogout(int fd)
{
    char re = (char)SUCCESS;
    send(fd, &re, 1, 0);
    instance->usernames.erase(fd);
}

void Server::processList(int fd)
{
    char buf[32];
    std::string requester = instance->usernames[fd];
    if(!instance->friends.count(requester) || instance->friends[requester].size() == 0)
    {
        sprintf(buf, "%d", 0);
        send(fd, buf, sizeof(int), 0);
        std::cout << "0 entries to send in total.\n\n";
        std::cout << "Finished sending.\n\n";
        return;
    }
    std::set<std::string>& names = instance->friends[requester];
    int cnt = names.size();
    sprintf(buf, "%d", cnt);
    send(fd, buf, sizeof(int), 0);
    std::cout << cnt << " entries to send in total.\n\n";
    std::set<std::string>::iterator iter;
    for(iter = names.begin(); iter != names.end(); iter++)
    {
        std::string name = *iter;
        buf[0] = (char)name.size();
        name.copy(buf + 1, name.size());
        send(fd, buf, 32, 0);
        std::cout << --cnt << " entries remaining to send.\n\n";
    }
    std::cout << "Finished sending.\n\n";
}

void Server::processSearch(int fd)
{
    int cnt = instance->usernames.size() - 1;
    char buf[32];
    sprintf(buf, "%d", cnt);
    send(fd, buf, sizeof(int), 0);
    std::map<int, std::string>::iterator iter;
    std::cout << cnt << " entries to send in total.\n\n";
    for(iter = instance->usernames.begin(); iter != instance->usernames.end(); iter++)
    {
        if(fd == iter->first)
        {
            continue;
        }
        std::string& name = iter->second;
        buf[0] = (char)(name.size());
        name.copy(buf + 1, name.size());
        send(fd, buf, 32, 0);
        std::cout << --cnt << " entries remaining to send.\n\n";
    }
    std::cout << "Finished sending.\n\n";
}

void Server::processLogin(int fd)
{
    char* buf = new char[sizeof(login_packet)];
    if(recv(fd, buf, sizeof(login_packet), 0) <= 0)
    {
        onConnectionClosed(fd);
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
        std::cout << "Username " << usrname.c_str() << "already online. Logging denied.\n\n";
        instance->usernames.erase(fd);
        return;
    }
    if(instance->passwords.count(usrname))
    {
        if(psword != instance->passwords[usrname])
        {
            re = (char)WRONG_PASSWORD;
            send(fd, &re, 1, 0);
            std::cout << "Username " << usrname.c_str() << " and password " << psword.c_str() << " don't match.\n\n";
            instance->usernames.erase(fd);
            instance->usersockets.erase(usrname);
            return;
        }
        re = (char)SUCCESS;
        send(fd, &re, 1, 0);
        std::cout << "IP " << instance->clientIPs[fd] << " logged in as user " << usrname.c_str() << ".\n\n";
        instance->usernames[fd] = usrname;
        instance->usersockets[usrname] = fd;
    }
    else
    {
        re = (char)ACCOUNT_CREATED;
        send(fd, &re, 1, 0);
        std::cout << "IP " << instance->clientIPs[fd] << " created account. Username: " << usrname.c_str() << ", password: " << psword.c_str() << ".\n\n";
        instance->usernames[fd] = usrname;
        instance->passwords[usrname] = psword;
        instance->usersockets[usrname] = fd;
    }
}

void Server::processAdd(int fd)
{
    char buf[32];
    if(recv(fd, buf, 32, 0) <= 0)
    {
        onConnectionClosed(fd);
    }
    int len = (int)buf[0];
    std::string name;
    name = name.assign(buf + 1, len);
    std::string requester = instance->usernames[fd];
    char re;
    if(requester == name)
    {
        re = (char)ADD_YOURSELF;
        send(fd, &re, 1, 0);
        std::cout << "User " << name << " tried to add himself as friend. Adding friend failed.\n\n";
    }
    else if(instance->passwords.count(name))
    {
        if(instance->friends.count(requester))
        {
            if(instance->friends[requester].count(name))
            {
                re = (char)ALREADY_FRIEND;
                send(fd, &re, 1, 0);
                std::cout << "User " << name << " is already friend of " << requester << ". Adding friend failed.\n\n";
            }
        }
        else
        {
            instance->friends[requester] = std::set<std::string>();
        }
        instance->friends[requester].insert(name);
        if(!instance->friends.count(name))
        {
            instance->friends[name] = std::set<std::string>();
        }
        instance->friends[name].insert(requester);
        re = (char)SUCCESS;
        send(fd, &re, 1, 0);
        std::cout << "User " << name << " added as friend of " << requester << ".\n\n";
    }
    else
    {
        re = (char)USER_NON_EXIST;
        send(fd, &re, 1, 0);
        std::cout << "User " << name << " does not exist. Adding friend failed.\n\n";
    }
}

void Server::onConnectionClosed(int fd)
{
    std::cout << "Connection with " << instance->clientIPs[fd] << " is closed.\n\n";
    instance->clientIPs.erase(fd);
    instance->threads.erase(fd);
    instance->clients.remove(fd);
    if(instance->usernames.count(fd))
    {
        instance->usersockets.erase(instance->usernames[fd]);
        instance->usernames.erase(fd);
    }
    pthread_exit(NULL);
}

void* Server::service_thread(void *p)
{
    int fd = *(int*)p;
    char action;
    while(true)
    {
        if(recv(fd, &action, 1, 0) <= 0)
        {
            onConnectionClosed(fd);
        }
        switch(action)
        {
        case ACTION_LOGIN:
            std::cout << "IP " << instance->clientIPs[fd] << " requested logging in.\n\n";
            processLogin(fd);
            break;
        case ACTION_LOGOUT:
            std::cout << "IP " << instance->clientIPs[fd] << " requested logging out.\n\n";
            processLogout(fd);
            break;
        case ACTION_SEARCH:
            std::cout << "IP " << instance->clientIPs[fd] << " requested searching.\n\n";
            processSearch(fd);
            break;
        case ACTION_ADD:
            std::cout << "IP " << instance->clientIPs[fd] << " requested adding friend.\n\n";
            processAdd(fd);
            break;
        case ACTION_LIST:
            std::cout << "IP " << instance->clientIPs[fd] << " requested listing.\n\n";
            processList(fd);
            break;
        default:
            std::cout << "Invalid actioin: " << (int)action << ".\n\n";
        }
    }
    std::cout.flush();
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
