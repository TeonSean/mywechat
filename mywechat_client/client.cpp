#include"client.h"

Client::Client(QObject *parent):
    QObject(parent)
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

void Client::trySearch(QVector<QString>* strings)
{
    sendAction(ACTION_SEARCH);
    char buffer[32];
    int cnt;
    if(recv(client, buffer, sizeof(int), 0) <= 0)
    {
        closeConnect();
        emit serverError();
    }
    sscanf(buffer, "%d", &cnt);
    std::cout << cnt << " other users in total.\n";
    std::cout.flush();
    while(cnt--)
    {
        if(recv(client, buffer, 32, 0) <= 0)
        {
            closeConnect();
            emit serverError();
        }
        int len = (int)buffer[0];
        std::string str;
        str = str.assign(buffer + 1, len);
        strings->push_back(QString::fromStdString(str));
    }
    emit searchFinished(SUCCESS, strings);
}

void Client::tryConnect(const char *ip, int port)
{
    close(client);
    client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    emit connectFinished(::connect(client, (sockaddr*)&addr, sizeof(addr)));
}

void Client::tryAdd(QString name)
{
    sendAction(ACTION_ADD);
    char buf[32];
    buf[0] = (char)name.size();
    name.toStdString().copy(buf + 1, name.size());
    send(client, buf, 32, 0);
    char re;
    if(recv(client, &re, 1, 0) <= 0)
    {
        closeConnect();
        emit serverError();
    }
    emit addFinished(re);
}

void Client::tryLogout()
{
    sendAction(ACTION_LOGOUT);
    char re;
    if(recv(client, &re, 1, 0) <= 0)
    {
        closeConnect();
        emit serverError();
    }
    emit logoutFinished(re);
}

void Client::tryLogin(QString name_, QString code_)
{
    sendAction(ACTION_LOGIN);
    std::string name = name_.toStdString();
    std::string code = code_.toStdString();
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
        emit serverError();
    }
    return loginFinished(re);
}

void Client::closeConnect()
{
    close(client);
}
