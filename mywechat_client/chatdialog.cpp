#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QThread>
#include <assert.h>

Reader::Reader(Client *client, std::string peername):
    client(client), peername(peername)
{

}

void Reader::run()
{
    while(true)
    {
        char* action = (char*)client->getNext(ACTION_RECV_MSG);
        if(*action == NEW_MSG)
        {
            std::string* name = (std::string*)client->getNext(ACTION_RECV_MSG);
            std::string* msg = (std::string*)client->getNext(ACTION_RECV_MSG);
            assert(*name == peername);
            emit newMsg(QString::fromStdString(*msg));
            delete name;
            delete msg;
        }
        delete action;
    }
}

ChatDialog::ChatDialog(Client &client, QString peername, QWidget *parent) :
    QDialog(parent), client(client), peername(peername),
    ui(new Ui::ChatDialog)
{
    ui->setupUi(this);
    ui->label->setText("Chatting with " + peername);
    thread = new Reader(&client, peername.toStdString());
    setWindowFlags(windowFlags() &~ Qt::WindowCloseButtonHint);
    connect(thread, SIGNAL(newMsg(QString)), this, SLOT(on_new_msg(QString)));
    connect(&client, SIGNAL(sendFinished(int,QString)), this, SLOT(on_send_finished(int,QString)));
    connect(this, SIGNAL(trySendMsg(QString,QString)), &client, SLOT(trySendMsg(QString,QString)));
    connect(&client, SIGNAL(exitFinished(int)), this, SLOT(on_exit_finished(int)));
    connect(this, SIGNAL(tryExit()), &client, SLOT(tryExit()));
    thread->start();
}

ChatDialog::~ChatDialog()
{
    delete thread;
    delete ui;
}

void ChatDialog::on_new_msg(QString str)
{
    ui->textBrowser->append(peername + ": " + str);
}

void ChatDialog::on_exit_finished(int re)
{
    ui->exitbtn->setEnabled(true);
    if(re != SUCCESS)
    {
        ui->textBrowser->append("Failed to exit chatting. Please try again.");
    }
    else
    {
        disconnect(thread, SIGNAL(newMsg(QString)), this, SLOT(on_new_msg(QString)));
        disconnect(&client, SIGNAL(sendFinished(int,QString)), this, SLOT(on_send_finished(int,QString)));
        disconnect(this, SIGNAL(trySendMsg(QString,QString)), &client, SLOT(trySendMsg(QString,QString)));
        disconnect(&client, SIGNAL(exitFinished(int)), this, SLOT(on_exit_finished(int)));
        disconnect(this, SIGNAL(tryExit()), &client, SLOT(tryExit()));
        client.tryExit();
        thread->terminate();
        close();
    }
}

void ChatDialog::on_send_finished(int re, QString text)
{
    ui->sendmsg->setEnabled(true);
    ui->sendfile->setEnabled(true);
    if(re == SUCCESS)
    {
        ui->textBrowser->append("You: " + text);
    }
    else
    {
        ui->textBrowser->append("(Sending failed) You: " + text);
    }
    ui->textEdit->setText("");
}

void ChatDialog::on_sendmsg_clicked()
{
    QString text = ui->textEdit->toPlainText();
    if(text.size() > 0)
    {
        ui->sendmsg->setEnabled(false);
        ui->sendfile->setEnabled(false);
        emit trySendMsg(peername, text);
    }
}

void ChatDialog::on_sendfile_clicked()
{

}

void ChatDialog::on_exitbtn_clicked()
{
    ui->exitbtn->setEnabled(false);
    emit tryExit();
}
