#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void initData();

public slots:
    void closeApp();
    void askClostApp();
    void aboutQt();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
