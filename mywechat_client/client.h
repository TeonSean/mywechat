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

class Client: public QObject
{
    Q_OBJECT

private:
    int client;

public:
    explicit Client(QObject *parent = 0);
    ~Client();

public slots:
    void tryConnect(const char* ip, int port);
    void closeConnect();
    void sendAction(int action);
    void tryLogin(QString name, QString code);
    void tryLogout();
    void trySearch(QVector<QString>* strings);

signals:
    void serverError();
    void connectFinished(int re);
    void loginFinished(int re);
    void logoutFinished(int re);
    void searchFinished(int re, QVector<QString>* strs);

};

#endif // CLIENT_H
