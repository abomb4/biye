#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public slots:
    void _do_login();

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

private:
    Ui::Login *ui;

};

#endif // DIALOG_H
