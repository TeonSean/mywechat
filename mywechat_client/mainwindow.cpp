#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverconfig.h"
#include "QMessageBox"
#include "login.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    onDisconnect();
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
}

void MainWindow::onDisconnect()
{
    connected = false;
    logged = false;
    ui->conn->setEnabled(true);
    ui->disconn->setEnabled(false);
    ui->login->setEnabled(false);
    ui->logout->setEnabled(false);
}

void MainWindow::onLogin()
{
    logged = true;
    ui->login->setEnabled(false);
    ui->logout->setEnabled(true);
}

void MainWindow::onLogout()
{
    logged = false;
    ui->login->setEnabled(true);
    ui->logout->setEnabled(false);
}

void MainWindow::showMessage(QString str)
{
    QMessageBox mb(this);
    mb.setText(str);
    mb.exec();
}

void MainWindow::on_login_clicked()
{
    assert(connected);
    assert(!logged);
    ui->login->setEnabled(false);
    Login* login = new Login(this);
    if(login->exec() == QDialog::Accepted)
    {
        std::string usrname, psword;
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
        int re = client.tryLogin(usrname, psword);
        switch(re)
        {
        case -1:
            showMessage("Server error.");
            onDisconnect();
            break;
        case SUCCEESS:
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
}

void MainWindow::on_conn_clicked()
{
    assert(!connected);
    ServerConfig* sc = new ServerConfig(this);
    if(sc->exec() == QDialog::Accepted)
    {
        if(client.tryConnect(sc->getIP().toStdString().c_str(), sc->getPort()) != -1)
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
}

void MainWindow::on_disconn_clicked()
{
    assert(connected);
    client.closeConnect();
    showMessage("Disconnected from server.");
    onDisconnect();
}

void MainWindow::on_logout_clicked()
{
    assert(connected);
    assert(logged);
    ui->logout->setEnabled(false);
    int re = client.tryLogout();
    switch(re)
    {
    case -1:
        showMessage("Server error.");
        onDisconnect();
        break;
    case SUCCEESS:
        showMessage("Log out succeess.");
        onLogout();
        break;
    default:
        showMessage("Unknown return code. Connection shut down.");
        onDisconnect();
        break;
    }
}
