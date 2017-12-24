#include"client.h"

Client::Client()
{

}

Client::~Client()
{
    close(client);
}

void Client::sendAction(int action)
{
    char c = action;
    send(client, &c, 1, 0);
}

int Client::tryConnect(const char *ip, int port)
{
    close(client);
    client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    return connect(client, (sockaddr*)&addr, sizeof(addr));
}

int Client::tryLogout()
{
    sendAction(ACTION_LOGOUT);
    char re;
    if(recv(client, &re, 1, 0) <= 0)
    {
        closeConnect();
        return -1;
    }
    return re;
}

int Client::tryLogin(std::string name, std::string code)
{
    sendAction(ACTION_LOGIN);
    login_packet p;
    p.namelen = (char)name.size();
    p.codelen = (char)code.size();
    name.copy(p.name, name.size());
    code.copy(p.code, code.size());
    send(client, &p, sizeof(login_packet), 0);
    char re;
    if(recv(client, &re, 1, 0) <= 0)
    {
        closeConnect();
        return -1;
    }
    return re;
}

void Client::closeConnect()
{
    close(client);
}
