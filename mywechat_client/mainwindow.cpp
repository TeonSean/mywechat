#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverconfig.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    connected(false)
{
    ui->setupUi(this);
    ui->pushButton_2->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showMessage(QString str)
{
    QMessageBox mb(this);
    mb.setText(str);
    mb.exec();
}

void MainWindow::on_pushButton_clicked()
{
    ServerConfig* sc = new ServerConfig(this);
    if(sc->exec() == QDialog::Accepted)
    {
        if(client.tryConnect(sc->getIP().toStdString().c_str(), sc->getPort()) != -1)
        {
            showMessage("Connection succeeded.");
            connected = true;
            ui->pushButton->setEnabled(false);
            ui->pushButton_2->setEnabled(true);
        }
        else
        {
            showMessage("Connection failed.");
            connected = false;
            ui->pushButton->setEnabled(true);
            ui->pushButton_2->setEnabled(false);
        }
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    client.closeConnect();
    showMessage("Disconnected from server.");
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(false);
}
