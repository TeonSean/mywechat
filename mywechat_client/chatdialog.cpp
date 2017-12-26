#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QThread>
#include <assert.h>
#include <QFileDialog>
#include <cstdio>

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
        else if(*action == NEW_FILE)
        {
            char* state = (char*)client->getNext(ACTION_RECV_FILE);
            assert(*state == NEW_FILE);
            std::string* name = (std::string*)client->getNext(ACTION_RECV_FILE);
            int* flen = (int*)client->getNext(ACTION_RECV_FILE);
            std::string* fname = (std::string*)client->getNext(ACTION_RECV_FILE);
            assert(*name == peername);
            emit newFile(*flen, QString::fromStdString(*fname));
            QString path = "/home/teon/Downloads/" + QString::fromStdString(*fname);
            FILE* f = fopen(path.toStdString().c_str(), "w");
            int remain = *flen;
            while(true)
            {
                if(remain > 1024)
                {
                    char* buf = (char*)client->getNext(ACTION_RECV_FILE);
                    fwrite(buf, 1, 1024, f);
                    remain -= 1024;
                    emit receiving(remain, *flen);
                    delete buf;
                }
                else
                {
                    char* buf = (char*)client->getNext(ACTION_RECV_FILE);
                    fwrite(buf, 1, remain, f);
                    delete buf;
                    break;
                }
            }
            fclose(f);
            emit receiveFinished();
            delete state;
            delete name;
            delete flen;
            delete fname;
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
    connect(thread, SIGNAL(newFile(int,QString)), this, SLOT(on_new_file(int,QString)));
    connect(thread, SIGNAL(receiving(int,int)), this, SLOT(on_receiving(int,int)));
    connect(thread, SIGNAL(receiveFinished()), this, SLOT(on_received_finished()));
    connect(&client, SIGNAL(sendFinished(int,QString)), this, SLOT(on_send_finished(int,QString)));
    connect(this, SIGNAL(trySendMsg(QString,QString)), &client, SLOT(trySendMsg(QString,QString)));
    connect(&client, SIGNAL(exitFinished(int)), this, SLOT(on_exit_finished(int)));
    connect(this, SIGNAL(tryExit()), &client, SLOT(tryExit()));
    connect(&client, SIGNAL(sendFileFinished(int,QString)), this, SLOT(on_send_file_finished(int,QString)));
    connect(this, SIGNAL(trySendFile(QString,QFile*)), &client, SLOT(trySendFile(QString,QFile*)));
    connect(&client, SIGNAL(sending(int,int)), this, SLOT(on_sending(int,int)));
    thread->start();
}

ChatDialog::~ChatDialog()
{
    delete thread;
    delete ui;
}

void ChatDialog::on_sending(int remain, int all)
{
    ui->textBrowser->setText(cache);
    ui->textBrowser->append("Uploading...\n" + QString::number(remain) + "/" + QString::number(all) + " bytes...");
}

void ChatDialog::on_new_msg(QString str)
{
    ui->textBrowser->append(peername + ": " + str);
}

void ChatDialog::on_new_file(int len, QString fname)
{
    ui->textBrowser->append("Started receiving file " + fname + " from " + peername + ", file size is " + QString::number(len) + " bytes.\nDownloading...");
    cache = ui->textBrowser->toPlainText();
}

void ChatDialog::on_received_finished()
{
    ui->textBrowser->setText(cache);
    ui->textBrowser->append("Download finished.");
}

void ChatDialog::on_receiving(int remain, int all)
{
    ui->textBrowser->setText(cache);
    ui->textBrowser->append(QString::number(remain) + "/" + QString::number(all) + " bytes...");
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
        disconnect(thread, SIGNAL(newFile(int,QString)), this, SLOT(on_new_file(int,QString)));
        disconnect(&client, SIGNAL(sendFinished(int,QString)), this, SLOT(on_send_finished(int,QString)));
        disconnect(this, SIGNAL(trySendMsg(QString,QString)), &client, SLOT(trySendMsg(QString,QString)));
        disconnect(&client, SIGNAL(exitFinished(int)), this, SLOT(on_exit_finished(int)));
        disconnect(this, SIGNAL(tryExit()), &client, SLOT(tryExit()));
        disconnect(&client, SIGNAL(sendFileFinished(int,QString)), this, SLOT(on_send_file_finished(int,QString)));
        disconnect(this, SIGNAL(trySendFile(QString,QFile*)), &client, SLOT(trySendFile(QString,QFile*)));
        client.tryExit();
        thread->terminate();
        close();
    }
}

void ChatDialog::on_send_file_finished(int re, QString fname)
{
    ui->sendmsg->setEnabled(true);
    ui->sendfile->setEnabled(true);
    if(re == SUCCESS)
    {
        ui->textBrowser->setText(cache + "File " + fname + " successfully sent.\n");
    }
    else
    {
        ui->textBrowser->setText(cache + "Failed to send file " + fname + ".\n");
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
    QFileDialog* fd = new QFileDialog(this);
    if(fd->exec() == QDialog::Accepted)
    {
        ui->sendmsg->setEnabled(false);
        ui->sendfile->setEnabled(false);
        QStringList slist = fd->selectedFiles();
        if(slist.size() != 1)
        {
            ui->sendmsg->setEnabled(true);
            ui->sendfile->setEnabled(true);
        }
        else
        {
            QFile* f = new QFile(slist[0]);
            emit trySendFile(peername, f);
            cache = ui->textBrowser->toPlainText();
        }
    }
}

void ChatDialog::on_exitbtn_clicked()
{
    ui->exitbtn->setEnabled(false);
    emit tryExit();
}
