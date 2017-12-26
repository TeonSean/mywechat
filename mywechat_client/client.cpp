#include"client.h"
#include<QFile>
#include<QFileInfo>

ReadThread::ReadThread(int fd, Client* client, pthread_mutex_t *mtx):
    QThread(NULL), fd(fd), client(client), mtx(mtx)
{

}

void ReadThread::run()
{
    std::cout << "Reader starts looping...\n";
    std::cout.flush();
    while(true)
    {
        char action = readAction();
        pthread_mutex_lock(mtx);
        switch(action)
        {
        case ACTION_LOGIN:
        case ACTION_LOGOUT:
        case ACTION_ADD:
        case ACTION_SEND_MSG:
        case ACTION_SEND_FILE:
        case ACTION_CHAT:
        case ACTION_EXIT:
        {
            char* response = new char;
            *response = readAction();
            client->queues[action].push(response);
            break;
        }
        case ACTION_SEARCH:
        case ACTION_LIST:
        {
            int* cnt = new int;
            *cnt = readInt();
            client->queues[action].push(cnt);
            for(int i = 0; i < *cnt; i++)
            {
                std::string* name = new std::string;
                *name = readName();
                client->queues[action].push(name);
            }
            break;
        }
        case ACTION_PROFILE:
        {
            std::string* name = new std::string;
            std::string* code = new std::string;
            *name = readName();
            *code = readName();
            client->queues[action].push(name);
            client->queues[action].push(code);
            break;
        }
        case ACTION_RECV_MSG:
        {
            char* state = new char;
            *state = readAction();
            client->queues[action].push(state);
            if(*state == NEW_MSG)
            {
                std::string* name = new std::string;
                std::string* msg = new std::string;
                *name = readName();
                *msg = readMessage();
                client->queues[action].push(name);
                client->queues[action].push(msg);
            }
            break;
        }
        case ACTION_RECV_FILE:
        {
            char* state = new char;
            *state = readAction();
            client->queues[action].push(state);
            if(*state == NEW_FILE)
            {
                std::string* sender = new std::string;
                int* flen = new int;
                std::string* fname = new std::string;
                *sender = readName();
                *flen = readInt();
                *fname = readFileName();
                client->queues[action].push(sender);
                client->queues[action].push(flen);
                client->queues[action].push(fname);
                int remain = *flen;
                while(true)
                {
                    if(remain > 1024)
                    {
                        char* buf = new char[1024];
                        if(recv(fd, buf, 1024, 0) <= 0)
                        {
                            emit serverError();
                        }
                        remain -= 1024;
                        client->queues[action].push(buf);
                    }
                    else
                    {
                        char* buf = new char[remain];
                        if(recv(fd, buf, remain, 0) <= 0)
                        {
                            emit serverError();
                        }
                        client->queues[action].push(buf);
                        break;
                    }
                }
            }
            break;
        }
        default:
        {
            std::cout << "Unknown action: " << (int)action << "\n\n";
            break;
        }
        }
        std::cout.flush();
        pthread_mutex_unlock(mtx);
    }
}

int ReadThread::readInt()
{
    int n;
    if(recv(fd, &n, sizeof(int), 0) <= 0)
    {
        emit serverError();
        return 0;
    }
    std::cout << "read int " << n << ".\n";
    return n;
}

char ReadThread::readAction()
{
    char c;
    if(recv(fd, &c, 1, 0) <= 0)
    {
        emit serverError();
        return -1;
    }
    std::cout << "read action " << (int)c << ".\n";
    return c;
}

std::string ReadThread::readFileName()
{
    char buf[128];
    if(recv(fd, buf, 128, 0) <= 0)
    {
        emit serverError();
        return "";
    }
    std::string re;
    re = re.assign(buf + sizeof(int), *(int*)buf);
    std::cout << "read file name " << re << ".\n";
    return re;
}

std::string ReadThread::readMessage()
{
    int n = readInt();
    char* buf = new char[n];
    if(recv(fd, buf, n, 0) <= 0)
    {
        emit serverError();
        return "";
    }
    std::string str;
    str = str.assign(buf, n);
    std::cout << "read msg " << str << ".\n";
    return str;
}

std::string ReadThread::readName()
{
    char buf[32];
    if(recv(fd, buf, 32, 0) <= 0)
    {
        emit serverError();
        return "";
    }
    std::string re;
    re = re.assign(buf + 1, (int)buf[0]);
    std::cout << "Read name " << re << ".\n";
    return re;
}

Client::Client(QObject *parent):
    QObject(parent)
{
    for(int i = 0 ; i <= ACTION_EXIT; i++)
    {
        queues.push_back(std::queue<void*>());
    }
    mtx = new pthread_mutex_t;
    pthread_mutex_init(mtx, NULL);
    reader = NULL;
}

Client::~Client()
{
    if(reader != NULL)
    {
        disconnect(reader, SIGNAL(serverError()), this, SIGNAL(serverError()));
        disconnect(reader, SIGNAL(serverError()), this, SLOT(closeConnect()));
        reader->exit();
        delete reader;
    }
    close(client);
    pthread_mutex_destroy(mtx);
    delete mtx;
}

void Client::sendFileName(std::string name)
{
    char buf[128];
    *(int*)buf = name.size();
    name.copy(buf + sizeof(int), name.size());
    send(client, buf, 128, 0);
    std::cout << "Sent file name " << name << ".\n";
}

void Client::sendMessage(std::string msg)
{
    sendInt(msg.size());
    send(client, msg.c_str(), msg.size(), 0);
    std::cout << "Sent msg " << msg << ".\n";
}

void Client::trySendFile(QString name, QFile* f)
{
    sendAction(ACTION_SEND_FILE);
    sendName(name.toStdString());
    sendInt(f->size());
    QFileInfo fi(f->fileName());
    sendFileName(fi.fileName().toStdString());
    FILE* fp = fopen(f->fileName().toStdString().c_str(), "r");
    char buf[1024];
    int remain = f->size();
    while(!feof(fp))
    {
        int actual = fread(buf, 1, 1024, fp);
        if(actual != 1024)
        {
            assert(actual == remain);
        }
        remain -= 1024;
        send(client, buf, actual, 0);
        emit sending(remain, f->size());
    }
    fclose(fp);
    char* action = (char*)getNext(ACTION_SEND_FILE);
    emit sendFileFinished((int)*action, fi.fileName());
    delete action;
}

void Client::tryReceiveFile()
{
    sendAction(ACTION_RECV_FILE);
    char* action = (char*)getNext(ACTION_RECV_FILE);
    if(*action == NOTHING_NEW)
    {
        emit noNewMsgFile();
    }
    else if(*action == NEW_FILE)
    {
        std::string* sender = (std::string*)getNext(ACTION_RECV_FILE);
        int* len = (int*)getNext(ACTION_RECV_FILE);
        std::string* fname = (std::string*)getNext(ACTION_RECV_FILE);
        emit newFile(QString::fromStdString(*sender), *len, QString::fromStdString(*fname));
        int remain = *len;
        QString path = "/home/teon/Downloads/" + QString::fromStdString(*fname);
        FILE* f = fopen(path.toStdString().c_str(), "w");
        while(true)
        {
            if(remain > 1024)
            {
                char* buf = (char*)getNext(ACTION_RECV_FILE);
                fwrite(buf, 1, 1024, f);
                remain -= 1024;
                emit receiving(remain, *len);
                delete buf;
            }
            else
            {
                char* buf = (char*)getNext(ACTION_RECV_FILE);
                fwrite(buf, 1, remain, f);
                delete buf;
                break;
            }
        }
        fclose(f);
        emit receiveFinished();
        delete fname;
        delete len;
        delete sender;
    }
    delete action;
}

void Client::tryReceiveMsg()
{
    sendAction(ACTION_RECV_MSG);
    char* action = (char*)getNext(ACTION_RECV_MSG);
    if(*action == NOTHING_NEW)
    {
        emit noNewMsgFile();
    }
    else if(*action == NEW_MSG)
    {
        std::string* name = (std::string*)getNext(ACTION_RECV_MSG);
        std::string* msg = (std::string*)getNext(ACTION_RECV_MSG);
        emit newMsg(QString::fromStdString(*name), QString::fromStdString(*msg));
        delete name;
        delete msg;
    }
    delete action;
}

void Client::tryExit()
{
    sendAction(ACTION_EXIT);
    char* re = (char*)getNext(ACTION_EXIT);
    emit exitFinished((int)*re);
    delete re;
}

void Client::trySendMsg(QString name, QString msg)
{
    sendAction(ACTION_SEND_MSG);
    sendName(name.toStdString());
    sendMessage(msg.toStdString());
    char* action = (char*)getNext(ACTION_SEND_MSG);
    emit sendFinished((int)*action, msg);
    delete action;
}

void Client::sendInt(int n)
{
    send(client, &n, sizeof(int), 0);
    std::cout << "Sent int " << n << ".\n";
}

void Client::sendAction(char action)
{
    send(client, &action, 1, 0);
    std::cout << "Sent action " << (int)action << ".\n";
}

void Client::sendName(std::string name)
{
    char buf[32];
    buf[0] = (char)name.size();
    name.copy(buf + 1, name.size());
    send(client, buf, 32, 0);
    std::cout << "Sent name " << name << ".\n";
}

void Client::tryProfile()
{
    sendAction(ACTION_PROFILE);
    std::string* name = (std::string*)getNext(ACTION_PROFILE);
    std::string* code = (std::string*)getNext(ACTION_PROFILE);
    emit profileFinished(QString::fromStdString(*name), QString::fromStdString(*code));
    delete name;
    delete code;
}

void Client::tryList(QVector<QString> *strings)
{
    sendAction(ACTION_LIST);
    int* cnt = (int*)getNext(ACTION_LIST);
    for(int i = 0; i < *cnt; i++)
    {
        std::string* name = (std::string*)getNext(ACTION_LIST);
        strings->push_back(QString::fromStdString(*name));
        delete name;
    }
    delete cnt;
    emit listFinished(strings);
}

void Client::trySearch(QVector<QString>* strings)
{
    sendAction(ACTION_SEARCH);
    int* cnt = (int*)getNext(ACTION_SEARCH);
    for(int i = 0; i < *cnt; i++)
    {
        std::string* name = (std::string*)getNext(ACTION_SEARCH);
        strings->push_back(QString::fromStdString(*name));
        delete name;
    }
    delete cnt;
    emit searchFinished(strings);
}

void Client::tryConnect(const char *ip, int port)
{
    close(client);
    client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int re = ::connect(client, (sockaddr*)&addr, sizeof(addr));
    emit connectFinished(re);
    if(re != -1)
    {
        if(reader != NULL)
        {
            disconnect(reader, SIGNAL(serverError()), this, SIGNAL(serverError()));
            disconnect(reader, SIGNAL(serverError()), this, SLOT(closeConnect()));
            reader->exit();
            delete reader;
        }
        reader = new ReadThread(client, this, mtx);
        connect(reader, SIGNAL(serverError()), this, SIGNAL(serverError()));
        connect(reader, SIGNAL(serverError()), this, SLOT(closeConnect()));
        reader->start();
    }
}

void* Client::getNext(char action)
{
    pthread_mutex_lock(mtx);
    while(queues[action].empty())
    {
        pthread_mutex_unlock(mtx);
        QThread::msleep(100);
        pthread_mutex_lock(mtx);
    }
    pthread_mutex_unlock(mtx);
    void* re = queues[action].front();
    queues[action].pop();
    return re;
}

void Client::tryAdd(QString name)
{
    sendAction(ACTION_ADD);
    sendName(name.toStdString());
    char* re = (char*)getNext(ACTION_ADD);
    emit addFinished((int)*re);
    delete re;
}

void Client::tryChat(QString name)
{
    sendAction(ACTION_CHAT);
    sendName(name.toStdString());
    char* re = (char*)getNext(ACTION_CHAT);
    emit chatFinished((int)*re, name);
    delete re;
}

void Client::tryLogout()
{
    sendAction(ACTION_LOGOUT);
    char* re = (char*)getNext(ACTION_LOGOUT);
    emit logoutFinished((int)*re);
    delete re;
}

void Client::tryLogin(QString name, QString code)
{
    sendAction(ACTION_LOGIN);
    sendName(name.toStdString());
    sendName(code.toStdString());
    char* re = (char*)getNext(ACTION_LOGIN);
    emit loginFinished((int)*re);
    delete re;
}

void Client::closeConnect()
{
    if(reader != NULL)
    {
        disconnect(reader, SIGNAL(serverError()), this, SIGNAL(serverError()));
        disconnect(reader, SIGNAL(serverError()), this, SLOT(closeConnect()));
        reader->exit();
        delete reader;
    }
    close(client);
}
