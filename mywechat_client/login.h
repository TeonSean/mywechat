#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    QString getUsername();
    QString getPassword();

private:
    Ui::Login *ui;
};

#endif // LOGIN_H
