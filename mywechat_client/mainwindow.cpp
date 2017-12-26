#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverconfig.h"
#include "usernameinput.h"
#include "QMessageBox"
#include "login.h"
#include "chatdialog.h"
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    onDisconnect();
    QThread* thread = new QThread();
    thread->start();
    setFixedSize(size());
    client.moveToThread(thread);
    connect(&client, SIGNAL(serverError()), this, SLOT(on_server_error()));
    connect(&client, SIGNAL(connectFinished(int)), this, SLOT(on_connect_finished(int)));
    connect(&client, SIGNAL(loginFinished(int)), this, SLOT(on_login_finished(int)));
    connect(&client, SIGNAL(logoutFinished(int)), this, SLOT(on_logout_finished(int)));
    connect(&client, SIGNAL(searchFinished(QVector<QString>*)), this, SLOT(on_search_finished(QVector<QString>*)));
    connect(&client, SIGNAL(addFinished(int)), this, SLOT(on_add_finished(int)));
    connect(&client, SIGNAL(listFinished(QVector<QString>*)), this, SLOT(on_list_finished(QVector<QString>*)));
    connect(&client, SIGNAL(profileFinished(QString,QString)), this, SLOT(on_profile_finished(QString,QString)));
    connect(&client, SIGNAL(chatFinished(int,QString)), this, SLOT(on_chat_finished(int,QString)));
    connect(&client, SIGNAL(newMsg(QString,QString)), this, SLOT(on_new_msg(QString,QString)));
    connect(&client, SIGNAL(noNewMsgFile()), this, SLOT(on_nothing_new()));
    connect(this, SIGNAL(closeConnect()), &client, SLOT(closeConnect()));
    connect(this, SIGNAL(tryConnect(const char*,int)), &client, SLOT(tryConnect(const char*,int)));
    connect(this, SIGNAL(tryLogin(QString,QString)), &client, SLOT(tryLogin(QString,QString)));
    connect(this, SIGNAL(tryLogout()), &client, SLOT(tryLogout()));
    connect(this, SIGNAL(trySearch(QVector<QString>*)), &client, SLOT(trySearch(QVector<QString>*)));
    connect(this, SIGNAL(tryAdd(QString)), &client, SLOT(tryAdd(QString)));
    connect(this, SIGNAL(tryList(QVector<QString>*)), &client, SLOT(tryList(QVector<QString>*)));
    connect(this, SIGNAL(tryProfile()), &client, SLOT(tryProfile()));
    connect(this, SIGNAL(tryChat(QString)), &client, SLOT(tryChat(QString)));
    connect(this, SIGNAL(tryReceiveMsg()), &client, SLOT(tryReceiveMsg()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnect()
{
    connected = true;
    ui->conn->setEnabled(false);
    ui->disconn->setEnabled(true);
    ui->login->setEnabled(true);
    ui->logout->setEnabled(false);
    ui->search->setEnabled(false);
    ui->add->setEnabled(false);
    ui->list->setEnabled(false);
    ui->prof->setEnabled(false);
    ui->chat->setEnabled(false);
    ui->recvfile->setEnabled(false);
    ui->recvmsg->setEnabled(false);
}

void MainWindow::onDisconnect()
{
    connected = false;
    logged = false;
    ui->conn->setEnabled(true);
    ui->disconn->setEnabled(false);
    ui->login->setEnabled(false);
    ui->logout->setEnabled(false);
    ui->search->setEnabled(false);
    ui->add->setEnabled(false);
    ui->list->setEnabled(false);
    ui->prof->setEnabled(false);
    ui->chat->setEnabled(false);
    ui->recvfile->setEnabled(false);
    ui->recvmsg->setEnabled(false);
}

void MainWindow::onLogin()
{
    logged = true;
    ui->login->setEnabled(false);
    ui->logout->setEnabled(true);
    ui->search->setEnabled(true);
    ui->add->setEnabled(true);
    ui->list->setEnabled(true);
    ui->prof->setEnabled(true);
    ui->chat->setEnabled(true);
    ui->recvfile->setEnabled(true);
    ui->recvmsg->setEnabled(true);
}

void MainWindow::onLogout()
{
    logged = false;
    ui->login->setEnabled(true);
    ui->logout->setEnabled(false);
    ui->search->setEnabled(false);
    ui->add->setEnabled(false);
    ui->list->setEnabled(false);
    ui->prof->setEnabled(false);
    ui->chat->setEnabled(false);
    ui->recvfile->setEnabled(false);
    ui->recvmsg->setEnabled(false);
}

void MainWindow::on_new_msg(QString name, QString msg)
{
    QString text = "New message from " + name + ":\n" + msg;
    ui->message->setText(text);
    ui->recvfile->setEnabled(true);
    ui->recvmsg->setEnabled(true);
}

void MainWindow::on_nothing_new()
{
    QString text = "You don't have any pending files or messages.";
    ui->message->setText(text);
    ui->recvfile->setEnabled(true);
    ui->recvmsg->setEnabled(true);
}

void MainWindow::showMessage(QString str)
{
    ui->message->setText(str);
}

void MainWindow::on_server_error()
{
    showMessage("Server error.");
    onDisconnect();
}

void MainWindow::on_login_clicked()
{
    assert(connected);
    assert(!logged);
    ui->login->setEnabled(false);
    Login* login = new Login(this);
    if(login->exec() == QDialog::Accepted)
    {
        QString usrname, psword;
        usrname = login->getUsername();
        psword = login->getPassword();
        if(usrname.size() == 0 || usrname.size() > 31)
        {
            ui->login->setEnabled(true);
            showMessage("Username and password lengths should be between 1 and 31.");
            return;
        }
        if(psword.size() == 0 || psword.size() > 31)
        {
            ui->login->setEnabled(true);
            showMessage("Username and password lengths should be between 1 and 31.");
            return;
        }
        emit tryLogin(usrname, psword);
    }
}

void MainWindow::on_login_finished(int re)
{
    switch(re)
    {
    case SUCCESS:
        showMessage("Log in succeess.");
        onLogin();
        break;
    case WRONG_PASSWORD:
        showMessage("Wrong password. Please try again.");
        onLogout();
        break;
    case ACCOUNT_CREATED:
        showMessage("Log in succeess. We have created an account for you.");
        onLogin();
        break;
    case ALREADY_ONLINE:
        showMessage("User already online. Logging denied.");
        onLogout();
        break;
    default:
        showMessage("Unknown return code. Connection shut down.");
        onDisconnect();
        break;
    }
}

void MainWindow::on_conn_clicked()
{
    assert(!connected);
    ServerConfig* sc = new ServerConfig(this);
    if(sc->exec() == QDialog::Accepted)
    {
        emit tryConnect(sc->getIP().toStdString().c_str(), sc->getPort());
    }
}

void MainWindow::on_connect_finished(int re)
{
    if(re != -1)
    {
        showMessage("Connection succeeded.");
        onConnect();
    }
    else
    {
        showMessage("Connection failed.");
        onDisconnect();
    }
}

void MainWindow::on_disconn_clicked()
{
    assert(connected);
    showMessage("Disconnected from server.");
    onDisconnect();
    emit closeConnect();
}

void MainWindow::on_logout_clicked()
{
    assert(connected);
    assert(logged);
    ui->logout->setEnabled(false);
    emit tryLogout();
}

void MainWindow::on_logout_finished(int re)
{
    switch(re)
    {
    case SUCCESS:
        showMessage("Log out succeess.");
        onLogout();
        break;
    default:
        showMessage("Unknown return code. Connection shut down.");
        onDisconnect();
        break;
    }
}

void MainWindow::on_search_clicked()
{
    assert(connected);
    assert(logged);
    ui->search->setEnabled(false);
    emit trySearch(new QVector<QString>());
}

void MainWindow::on_search_finished(QVector<QString>* strs)
{
    ui->search->setEnabled(true);
    QStandardItemModel* model = new QStandardItemModel(this);
    for(int i = 0; i < strs->size(); i++)
    {
        QStandardItem* item = new QStandardItem((*strs)[i]);
        model->appendRow(item);
    }
    ui->users->setModel(model);
    delete strs;
}

void MainWindow::on_add_finished(int re)
{
    ui->add->setEnabled(true);
    switch(re)
    {
    case SUCCESS:
        showMessage("Adding friend success.");
        ui->list->setEnabled(false);
        emit tryList(new QVector<QString>());
        break;
    case ADD_YOURSELF:
        showMessage("You cannot add yourself as your friend.");
        break;
    case ALREADY_FRIEND:
        showMessage("This user is already your friend.");
        break;
    case USER_NON_EXIST:
        showMessage("This user does not exist.");
        break;
    default:
        showMessage("Unknown return code. Connection shut down.");
        onDisconnect();
        break;
    }
}

void MainWindow::on_add_clicked()
{
    assert(connected);
    assert(logged);
    UsernameInput* uni = new UsernameInput(this);
    ui->add->setEnabled(false);
    if(uni->exec() == QDialog::Accepted)
    {
        if(uni->getUsername().size() == 0 || uni->getUsername().size() > 31)
        {
            ui->add->setEnabled(true);
            showMessage("Username length should be between 1 and 31.");
            return;
        }
        emit tryAdd(uni->getUsername());
    }
}

void MainWindow::on_profile_finished(QString name, QString code)
{
    QString text("");
    text = "Your profile:\nUsername: " + name + "\nPassword: " + code + "\n";
    ui->prof->setEnabled(true);
    ui->message->setText(text);
}

void MainWindow::on_list_finished(QVector<QString> *strs)
{
    ui->list->setEnabled(true);
    QStandardItemModel* model = new QStandardItemModel(this);
    for(int i = 0; i < strs->size(); i++)
    {
        QStandardItem* item = new QStandardItem((*strs)[i]);
        model->appendRow(item);
    }
    ui->friends->setModel(model);
    delete strs;
}

void MainWindow::on_list_clicked()
{
    assert(connected);
    assert(logged);
    ui->list->setEnabled(false);
    emit tryList(new QVector<QString>());
}

void MainWindow::on_prof_clicked()
{
    assert(connected);
    assert(logged);
    ui->prof->setEnabled(false);
    emit tryProfile();
}

void MainWindow::on_chat_finished(int re, QString name)
{
    switch(re)
    {
    case SUCCESS:
    {
        ui->chat->setEnabled(true);
        ChatDialog* cw = new ChatDialog(client, name, this);
        cw->exec();
        break;
    }
    case NOT_YOUR_FRIEND:
        showMessage("User " + name + " is not your friend.");
        ui->chat->setEnabled(true);
        break;
    default:
        showMessage("Unknown return code. Connection shut down.");
        onDisconnect();
        break;
    }
}

void MainWindow::on_chat_clicked()
{
    assert(connected);
    assert(logged);
    UsernameInput* uni = new UsernameInput(this);
    ui->chat->setEnabled(false);
    if(uni->exec() == QDialog::Accepted)
    {
        if(uni->getUsername().size() == 0 || uni->getUsername().size() > 31)
        {
            showMessage("Username length should be between 1 and 31.");
            return;
        }
        client.tryChat(uni->getUsername());
    }
}

void MainWindow::on_recvmsg_clicked()
{
    assert(connected);
    assert(logged);
    ui->recvmsg->setEnabled(false);
    ui->recvfile->setEnabled(false);
    emit tryReceiveMsg();
}
