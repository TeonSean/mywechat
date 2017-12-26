#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.h"
#include "messagedef.h"
#include "assert.h"
#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_login_clicked();

    void on_conn_clicked();

    void on_disconn_clicked();

    void on_logout_clicked();

    void on_search_clicked();

    void on_server_error();

    void on_connect_finished(int re);

    void on_login_finished(int re);

    void on_logout_finished(int re);

    void on_search_finished(QVector<QString>* strs);

    void on_add_finished(int re);

    void on_add_clicked();

    void on_list_finished(QVector<QString>* strs);

    void on_list_clicked();

    void on_profile_finished(QString name, QString code);

    void on_prof_clicked();

    void on_chat_clicked();

    void on_chat_finished(int re, QString name);

    void on_recvmsg_clicked();

    void on_new_msg(QString name, QString msg);

    void on_new_file(QString sender, int flen, QString fname);

    void on_nothing_new();

    void on_recvfile_clicked();

    void on_receiving(int remain, int all);

    void on_receive_finished();

private:
    Ui::MainWindow *ui;
    Client client;
    bool connected;
    bool logged;
    QString cache;

    void onConnect();
    void onDisconnect();
    void onLogin();
    void onLogout();
    void showMessage(QString str);

signals:
    void tryConnect(const char* ip, int port);
    void closeConnect();
    void tryLogin(QString name, QString code);
    void tryLogout();
    void trySearch(QVector<QString>* strings);
    void tryAdd(QString name);
    void tryList(QVector<QString>* strings);
    void tryProfile();
    void tryChat(QString name);
    void tryReceiveMsg();
    void tryReceiveFile();
};

#endif // MAINWINDOW_H
