#ifndef USERNAMEINPUT_H
#define USERNAMEINPUT_H

#include <QDialog>

namespace Ui {
class UsernameInput;
}

class UsernameInput : public QDialog
{
    Q_OBJECT

public:
    explicit UsernameInput(QWidget *parent = 0);
    ~UsernameInput();

    QString getUsername();

private:
    Ui::UsernameInput *ui;
};

#endif // USERNAMEINPUT_H
