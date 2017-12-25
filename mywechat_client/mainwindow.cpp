#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverconfig.h"
#include "usernameinput.h"
#include "QMessageBox"
#include "login.h"
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
    connect(&client, SIGNAL(searchFinished(int,QVector<QString>*)), this, SLOT(on_search_finished(int,QVector<QString>*)));
    connect(&client, SIGNAL(addFinished(int)), this, SLOT(on_add_finished(int)));
    connect(this, SIGNAL(closeConnect()), &client, SLOT(closeConnect()));
    connect(this, SIGNAL(tryConnect(const char*,int)), &client, SLOT(tryConnect(const char*,int)));
    connect(this, SIGNAL(tryLogin(QString,QString)), &client, SLOT(tryLogin(QString,QString)));
    connect(this, SIGNAL(tryLogout()), &client, SLOT(tryLogout()));
    connect(this, SIGNAL(trySearch(QVector<QString>*)), &client, SLOT(trySearch(QVector<QString>*)));
    connect(this, SIGNAL(tryAdd(QString)), &client, SLOT(tryAdd(QString)));
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
}

void MainWindow::onLogin()
{
    logged = true;
    ui->login->setEnabled(false);
    ui->logout->setEnabled(true);
    ui->search->setEnabled(true);
    ui->add->setEnabled(true);
}

void MainWindow::onLogout()
{
    logged = false;
    ui->login->setEnabled(true);
    ui->logout->setEnabled(false);
    ui->search->setEnabled(false);
    ui->add->setEnabled(false);
}

void MainWindow::showMessage(QString str)
{
    QMessageBox mb(this);
    mb.setText(str);
    mb.exec();
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
            showMessage("Username and password lengths should be between 1 and 31.");
            return;
        }
        if(psword.size() == 0 || psword.size() > 31)
        {
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
    QVector<QString>* strs = new QVector<QString>;
    emit trySearch(strs);
}

void MainWindow::on_search_finished(int re, QVector<QString>* strs)
{
    switch(re)
    {
    case SUCCESS:
    {
        ui->search->setEnabled(true);
        QStandardItemModel* model = new QStandardItemModel(this);
        for(int i = 0; i < strs->size(); i++)
        {
            QStandardItem* item = new QStandardItem((*strs)[i]);
            model->appendRow(item);
        }
        ui->users->setModel(model);
        break;
    }
    default:
        showMessage("Unknown return code. Connection shut down.");
        onDisconnect();
        break;
    }
    delete strs;
}

void MainWindow::on_add_finished(int re)
{
    ui->add->setEnabled(true);
    switch(re)
    {
    case SUCCESS:
        showMessage("Adding friend success.");
        emit tryList();
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
            showMessage("Username length should be between 1 and 31.");
            return;
        }
        emit tryAdd(uni->getUsername());
    }
}
