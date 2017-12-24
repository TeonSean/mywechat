#include "serverconfig.h"
#include "ui_serverconfig.h"

ServerConfig::ServerConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfig)
{
    ui->setupUi(this);
}

ServerConfig::~ServerConfig()
{
    delete ui;
}

QString ServerConfig::getIP()
{
    return ui->lineEdit->text();
}

int ServerConfig::getPort()
{
    return ui->lineEdit_2->text().toInt();
}
