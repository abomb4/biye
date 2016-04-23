#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->action_exit, SIGNAL(triggered(bool)), this, SLOT(askClostApp()));
    connect(ui->action_about_qt, SIGNAL(triggered(bool)), this, SLOT(aboutQt()));
}

MainWindow::~MainWindow() {
    delete ui;
}

//
void MainWindow::initData() {
    // TODO 1 get all user list from db
    //      2 get diff between local db and server from server
    //      3 get recent list from db
    //      4 get online list from server
}

void MainWindow::closeApp() {
    this->close();
}

void MainWindow::askClostApp() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("退出确认");
    msgBox.setText("确认退出程序？");
    msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    switch (msgBox.exec()) {
    case QMessageBox::Yes:
        this->closeApp();
        break;
    case QMessageBox::No:
    default:
        break;
    }
}
void MainWindow::aboutQt() {
    QMessageBox::aboutQt(NULL,"AboutQt");
}
