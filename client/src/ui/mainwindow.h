#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QListWidgetItem>

#include "structs/user.h"
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
    void initUi();

public slots:
    void closeApp();
    void askClostApp();
    void aboutQt();
    void openChatWindow(QListWidgetItem *item);
private:
    Ui::MainWindow *ui;
    QVector<User> *_users;
    QVector<uint32_t> *_recent;
};

class ContactItem : public QListWidgetItem {
public:
    ContactItem(QListWidget *view = Q_NULLPTR, int type = Type) : QListWidgetItem(view, type) { }
    ContactItem(const QString &text, QListWidget *view = Q_NULLPTR, int type = Type)
        : QListWidgetItem(text, view, type) {}

    ContactItem(const QIcon &icon, const QString &text,
                             QListWidget *view = Q_NULLPTR, int type = Type)
        : QListWidgetItem(icon, text, view, type) {}

    ContactItem(const QListWidgetItem &other) : QListWidgetItem(other) {}

    virtual ~ContactItem() {
        this->~QListWidgetItem();
    }
    enum DataRole : int {UserId = 10000};

    QVariant data(int role) const;
    void setData(int role, const QVariant & value);
private:
    uint32_t userid;
};

#endif // MAINWINDOW_H
