#include "usernameinput.h"
#include "ui_usernameinput.h"

UsernameInput::UsernameInput(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UsernameInput)
{
    ui->setupUi(this);
}

UsernameInput::~UsernameInput()
{
    delete ui;
}

QString UsernameInput::getUsername()
{
    return ui->lineEdit->text();
}
