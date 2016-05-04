#ifndef CHATINGWINDOW_H
#define CHATINGWINDOW_H

#include <QMainWindow>

namespace Ui {
class ChatingWindow;
}

class ChatingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatingWindow(QWidget *parent = 0);
    ~ChatingWindow();

private:
    Ui::ChatingWindow *ui;
};

#endif // CHATINGWINDOW_H
