#include "chatingwindow.h"
#include "ui_chatingwindow.h"

ChatingWindow::ChatingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatingWindow)
{
    ui->setupUi(this);
}

ChatingWindow::~ChatingWindow()
{
    delete ui;
}
