#ifndef CLIENT_H
#define CLIENT_H

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include "messagedef.h"
#include <string>
#include <QVector>
#include <QObject>
#include <QString>
#include <iostream>
#include <QThread>
#include <queue>

class Client;
class Reader;

class ReadThread: public QThread
{
    Q_OBJECT

private:
    int fd;
    Client* client;
    pthread_mutex_t* mtx;

public:
    explicit ReadThread(int fd, Client* client, pthread_mutex_t* mtx);

protected:
    virtual void run() Q_DECL_OVERRIDE;
    std::string readMessage();
    char readAction();
    int readInt();
    std::string readName();

signals:
    void serverError();
};

class Client: public QObject
{
    Q_OBJECT

    friend class ReadThread;
    friend class Reader;

private:
    int client;
    std::vector<std::queue<void*> > queues;
    ReadThread* reader;
    pthread_mutex_t* mtx;

    void* getNext(char action);

public:
    explicit Client(QObject *parent = 0);
    ~Client();

public slots:
    void tryConnect(const char* ip, int port);
    void closeConnect();
    void sendAction(char action);
    void sendName(std::string name);
    void sendInt(int n);
    void sendMessage(std::string msg);
    void tryLogin(QString name, QString code);
    void tryLogout();
    void trySearch(QVector<QString>* strings);
    void tryAdd(QString name);
    void tryList(QVector<QString>* strings);
    void tryProfile();
    void tryChat(QString name);
    void trySendMsg(QString name, QString msg);
    void tryExit();
    void tryReceiveMsg();

signals:
    void serverError();
    void connectFinished(int re);
    void loginFinished(int re);
    void logoutFinished(int re);
    void searchFinished(QVector<QString>* strs);
    void addFinished(int re);
    void listFinished(QVector<QString>* strings);
    void profileFinished(QString name, QString code);
    void chatFinished(int re, QString name);
    void sendFinished(int re, QString text);
    void exitFinished(int re);
    void newMsg(QString sender, QString msg);
    void noNewMsgFile();

};

#endif // CLIENT_H
