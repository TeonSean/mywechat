#include"client.h"

Client::Client()
{

}

Client::~Client()
{
    close(client);
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

void Client::closeConnect()
{
    close(client);
}
