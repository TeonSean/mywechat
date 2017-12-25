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

QString Login::getUsername()
{
    return ui->lineEdit->text();
}

QString Login::getPassword()
{
    return ui->lineEdit_2->text();
}
