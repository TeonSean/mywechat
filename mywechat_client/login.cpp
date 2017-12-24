#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
}

Login::~Login()
{
    delete ui;
}

std::string Login::getUsername()
{
    return ui->lineEdit->text().toStdString();
}

std::string Login::getPassword()
{
    return ui->lineEdit_2->text().toStdString();
}
