#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <client.h>

namespace Ui {
class ChatDialog;
}

class Reader: public QThread
{
    Q_OBJECT

private:
    Client* client;
    std::string peername;

public:
    explicit Reader(Client* client, std::string peername);

protected:
    virtual void run() Q_DECL_OVERRIDE;

signals:
    void newMsg(QString msg);
};

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(Client& client, QString peername, QWidget *parent = 0);
    ~ChatDialog();

private slots:
    void on_sendmsg_clicked();

    void on_sendfile_clicked();

    void on_send_finished(int re, QString text);

    void on_exitbtn_clicked();

    void on_exit_finished(int re);

    void on_new_msg(QString str);

private:
    Ui::ChatDialog *ui;
    Client& client;
    QString peername;
    Reader* thread;

signals:
    void trySendMsg(QString name, QString msg);
    void tryExit();
};

#endif // CHATDIALOG_H
