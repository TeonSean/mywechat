#include "serverconfig.h"
#include "ui_serverconfig.h"

ServerConfig::ServerConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfig)
{
    ui->setupUi(this);
    ui->lineEdit->setText("183.172.147.180");
    ui->lineEdit_2->setText("8088");
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
