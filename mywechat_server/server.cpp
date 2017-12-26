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
    memset(&unsent_lock, 0, sizeof(pthread_mutex_t));
    pthread_mutex_init(&unsent_lock, NULL);
    pthread_mutex_init(&chatting_lock, NULL);
}

void Server::sendInt(int fd, int n)
{
    std::cout << "Sent int " << n << ".\n";
    send(fd, &n, sizeof(int), 0);
}

void Server::sendMessage(int fd, std::string msg)
{
    sendInt(fd, msg.size());
    std::cout << "Sent msg " << msg << ".\n";
    send(fd, msg.c_str(), msg.size(), 0);
}

void Server::sendAction(int fd, char action)
{
    std::cout << "Sent action " << (int)action << ".\n";
    send(fd, &action, 1, 0);
}

void Server::sendName(int fd, std::string name)
{
    std::cout << "Sent name " << name << ".\n";
    char buf[32];
    buf[0] = (char)name.size();
    name.copy(buf + 1, name.size());
    send(fd, buf, 32, 0);
}

std::string Server::receiveMessage(int fd)
{
    int n = receiveInt(fd);
    char* buf = new char[n];
    recv(fd, buf, n, 0);
    std::string str;
    str = str.assign(buf, n);
    std::cout << "Received msg " << str << ".\n";
    return str;
}

int Server::receiveInt(int fd)
{
    int n;
    if(recv(fd, &n, sizeof(int), 0) <= 0)
    {
        onConnectionClosed(fd);
        return 0;
    }
    std::cout << "Received int " << n << ".\n";
    return n;
}

char Server::receiveAction(int fd)
{
    char c;
    if(recv(fd, &c, 1, 0) <= 0)
    {
        onConnectionClosed(fd);
        return -1;
    }
    std::cout << "Received action " << (int)c << ".\n";
    return c;
}

std::string Server::receiveName(int fd)
{
    char buf[32];
    if(recv(fd, buf, 32, 0) <= 0)
    {
        onConnectionClosed(fd);
        return "";
    }
    std::string re;
    re = re.assign(buf + 1, (int)buf[0]);
    std::cout << "Received name " << re << ".\n";
    return re;
}

void Server::processLogout(int fd)
{
    std::string name = usernames[fd];
    usernames.erase(fd);
    usersockets.erase(name);
    pthread_mutex_lock(&chatting_lock);
    chattingWith.erase(name);
    pthread_mutex_unlock(&chatting_lock);
    sendAction(fd, ACTION_LOGOUT);
    sendAction(fd, SUCCESS);
}

void Server::processList(int fd)
{
    std::string requester = usernames[fd];
    if(!friends.count(requester) || friends[requester].size() == 0)
    {
        sendAction(fd, ACTION_LIST);
        sendInt(fd, 0);
        std::cout << "0 entries to send in total.\n\n";
        std::cout << "Finished sending.\n\n";
        return;
    }
    std::set<std::string>& names = friends[requester];
    int cnt = names.size();
    sendAction(fd, ACTION_LIST);
    sendInt(fd, cnt);
    std::cout << cnt << " entries to send in total.\n\n";
    std::set<std::string>::iterator iter;
    for(iter = names.begin(); iter != names.end(); iter++)
    {
        std::string name = *iter;
        sendName(fd, name);
        std::cout << --cnt << " entries remaining to send.\n\n";
    }
    std::cout << "Finished sending.\n\n";
}

void Server::processSearch(int fd)
{
    int cnt = passwords.size() - 1;
    sendAction(fd, ACTION_SEARCH);
    sendInt(fd, cnt);
    std::map<std::string, std::string>::iterator iter;
    std::cout << cnt << " entries to send in total.\n\n";
    for(iter = passwords.begin(); iter != passwords.end(); iter++)
    {
        if(usernames[fd] == iter->first)
        {
            continue;
        }
        sendName(fd, iter->first);
        std::cout << --cnt << " entries remaining to send.\n\n";
    }
    std::cout << "Finished sending.\n\n";
}

void Server::processLogin(int fd)
{
    std::string usrname, psword;
    usrname = receiveName(fd);
    psword = receiveName(fd);
    char re;
    if(usersockets.count(usrname))
    {
        sendAction(fd, ACTION_LOGIN);
        sendAction(fd, ALREADY_ONLINE);
        std::cout << "Username " << usrname.c_str() << "already online. Logging denied.\n\n";
        usernames.erase(fd);
        return;
    }
    if(passwords.count(usrname))
    {
        if(psword != passwords[usrname])
        {
            sendAction(fd, ACTION_LOGIN);
            sendAction(fd, WRONG_PASSWORD);
            std::cout << "Username " << usrname.c_str() << " and password " << psword.c_str() << " don't match.\n\n";
            usernames.erase(fd);
            usersockets.erase(usrname);
            return;
        }
        sendAction(fd, ACTION_LOGIN);
        sendAction(fd, SUCCESS);
        std::cout << "IP " << clientIPs[fd] << " logged in as user " << usrname.c_str() << ".\n\n";
        usernames[fd] = usrname;
        usersockets[usrname] = fd;
    }
    else
    {
        sendAction(fd, ACTION_LOGIN);
        sendAction(fd, ACCOUNT_CREATED);
        std::cout << "IP " << clientIPs[fd] << " created account. Username: " << usrname.c_str() << ", password: " << psword.c_str() << ".\n\n";
        usernames[fd] = usrname;
        passwords[usrname] = psword;
        usersockets[usrname] = fd;
    }
}

void Server::processChat(int fd)
{
    std::string name = receiveName(fd);
    std::string requester = usernames[fd];
    if(friends.count(requester) && friends[requester].count(name))
    {
        pthread_mutex_lock(&chatting_lock);
        chattingWith[requester] = name;
        sendAction(fd, ACTION_CHAT);
        sendAction(fd, SUCCESS);
        std::cout << "User " << requester << " started chatting with " << name << ".\n\n";
        pthread_mutex_lock(&unsent_lock);
        if(unsent_msgs.count(requester))
        {
            for(std::list<unsent*>::iterator iter = unsent_msgs[requester].begin();
                iter != unsent_msgs[requester].end();)
            {
                unsent* un = *iter;
                if(un->sender == name)
                {
                    sendAction(fd, ACTION_RECV_MSG);
                    sendAction(fd, NEW_MSG);
                    sendName(fd, name);
                    sendMessage(fd, un->msg);
                    std::cout << "Cached message forwarded to " << requester << ".\n";
                    unsent_msgs[requester].erase(iter++);
                }
                else
                {
                    iter++;
                }
            }
        }
        pthread_mutex_unlock(&unsent_lock);
        pthread_mutex_unlock(&chatting_lock);
    }
    else
    {
        sendAction(fd, ACTION_CHAT);
        sendAction(fd, NOT_YOUR_FRIEND);
        std::cout << "User " << requester << " does not have friend " << name << ". Chatting failed.\n\n";
    }
}

void Server::processRecvMsg(int fd)
{
    std::string requester = usernames[fd];
    pthread_mutex_lock(&unsent_lock);
    if(!unsent_msgs.count(requester) || unsent_msgs[requester].size() == 0)
    {
        sendAction(fd, ACTION_RECV_MSG);
        sendAction(fd, NOTHING_NEW);
        pthread_mutex_unlock(&unsent_lock);
        return;
    }
    sendAction(fd, ACTION_RECV_MSG);
    sendAction(fd, NEW_MSG);
    unsent* un = unsent_msgs[requester].back();
    unsent_msgs[requester].pop_back();
    sendName(fd, un->sender);
    sendMessage(fd, un->msg);
    std::cout << "Cached message forwarded to " << requester << ".\n\n";
    delete un;
    pthread_mutex_unlock(&unsent_lock);
}

void Server::processAdd(int fd)
{
    std::string name = receiveName(fd);
    std::string requester = usernames[fd];
    if(requester == name)
    {
        sendAction(fd, ACTION_ADD);
        sendAction(fd, ADD_YOURSELF);
        std::cout << "User " << name << " tried to add himself as friend. Adding friend failed.\n\n";
    }
    else if(passwords.count(name))
    {
        if(friends.count(requester))
        {
            if(friends[requester].count(name))
            {
                sendAction(fd, ACTION_ADD);
                sendAction(fd, ALREADY_FRIEND);
                std::cout << "User " << name << " is already friend of " << requester << ". Adding friend failed.\n\n";
            }
        }
        else
        {
            friends[requester] = std::set<std::string>();
        }
        friends[requester].insert(name);
        if(!friends.count(name))
        {
            friends[name] = std::set<std::string>();
        }
        friends[name].insert(requester);
        sendAction(fd, ACTION_ADD);
        sendAction(fd, SUCCESS);
        std::cout << "User " << name << " added as friend of " << requester << ".\n\n";
    }
    else
    {
        sendAction(fd, ACTION_ADD);
        sendAction(fd, USER_NON_EXIST);
        std::cout << "User " << name << " does not exist. Adding friend failed.\n\n";
    }
}

void Server::processProfile(int fd)
{
    std::string requester = usernames[fd];
    std::string psword = passwords[requester];
    sendAction(fd, ACTION_PROFILE);
    sendName(fd, requester);
    sendName(fd, psword);
    std::cout << "Profile sent to user.\n\n";
}

void Server::onConnectionClosed(int fd)
{
    std::cout << "Connection with " << clientIPs[fd] << " is closed.\n\n";
    clientIPs.erase(fd);
    threads.erase(fd);
    clients.remove(fd);
    if(usernames.count(fd))
    {
        pthread_mutex_lock(&chatting_lock);
        chattingWith.erase(usernames[fd]);
        usersockets.erase(usernames[fd]);
        usernames.erase(fd);
        pthread_mutex_unlock(&chatting_lock);
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
            instance->onConnectionClosed(fd);
        }
        switch(action)
        {
        case ACTION_LOGIN:
            std::cout << "IP " << instance->clientIPs[fd] << " requested logging in.\n\n";
            instance->processLogin(fd);
            break;
        case ACTION_LOGOUT:
            std::cout << "IP " << instance->clientIPs[fd] << " requested logging out.\n\n";
            instance->processLogout(fd);
            break;
        case ACTION_SEARCH:
            std::cout << "IP " << instance->clientIPs[fd] << " requested searching.\n\n";
            instance->processSearch(fd);
            break;
        case ACTION_ADD:
            std::cout << "IP " << instance->clientIPs[fd] << " requested adding friend.\n\n";
            instance->processAdd(fd);
            break;
        case ACTION_LIST:
            std::cout << "IP " << instance->clientIPs[fd] << " requested listing.\n\n";
            instance->processList(fd);
            break;
        case ACTION_PROFILE:
            std::cout << "IP " << instance->clientIPs[fd] << " requested profiling.\n\n";
            instance->processProfile(fd);
            break;
        case ACTION_CHAT:
            std::cout << "IP " << instance->clientIPs[fd] << " requested chatting.\n\n";
            instance->processChat(fd);
            break;
        case ACTION_SEND_MSG:
            std::cout << "IP " << instance->clientIPs[fd] << " requested sending message.\n\n";
            instance->processSendMsg(fd);
            break;
        case ACTION_EXIT:
            std::cout << "IP " << instance->clientIPs[fd] << " requested exiting chatting.\n\n";
            instance->processExit(fd);
            break;
        case ACTION_RECV_MSG:
            std::cout << "IP " << instance->clientIPs[fd] << " requested receving message.\n\n";
            instance->processRecvMsg(fd);
            break;
        default:
            std::cout << "Invalid action: " << (int)action << ".\n\n";
        }
    }
    std::cout.flush();
}

void Server::processExit(int fd)
{
    pthread_mutex_lock(&chatting_lock);
    chattingWith.erase(usernames[fd]);
    sendAction(fd, ACTION_EXIT);
    sendAction(fd, SUCCESS);
    pthread_mutex_unlock(&chatting_lock);
}

void Server::processSendMsg(int fd)
{
    std::string name = receiveName(fd);
    std::string requester = usernames[fd];
    std::cout << requester << " attempts to send a message to " << name << ".\n";
    std::string msg = receiveMessage(fd);
    std::cout << "All data received.\n";
    sendAction(fd, ACTION_SEND_MSG);
    sendAction(fd, SUCCESS);
    pthread_mutex_lock(&chatting_lock);
    if(chattingWith.count(name) && chattingWith[name] == requester)
    {
        int fd2 = usersockets[name];
        sendAction(fd2, ACTION_RECV_MSG);
        sendAction(fd2, NEW_MSG);
        sendName(fd2, requester);
        sendMessage(fd2, msg);
        std::cout << "Data forwarded to " << name << " directly.\n";
        std::cout << "All data sent.\n\n";
    }
    else
    {
        pthread_mutex_lock(&unsent_lock);
        if(!unsent_msgs.count(name))
        {
            unsent_msgs[name] = std::list<unsent*>();
        }
        unsent* un = new unsent();
        un->sender = requester;
        un->msg = msg;
        unsent_msgs[name].push_back(un);
        pthread_mutex_unlock(&unsent_lock);
        std::cout << "Unsent message stored. Will be forwarded to " << name << " later.\n\n";
    }
    pthread_mutex_unlock(&chatting_lock);
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
