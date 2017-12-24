#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.h"
#include "messagedef.h"
#include "assert.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_login_clicked();

    void on_conn_clicked();

    void on_disconn_clicked();

    void on_logout_clicked();

private:
    Ui::MainWindow *ui;
    Client client;
    bool connected;
    bool logged;

    void onConnect();
    void onDisconnect();
    void onLogin();
    void onLogout();
    void showMessage(QString str);
};

#endif // MAINWINDOW_H
