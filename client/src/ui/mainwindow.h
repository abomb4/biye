#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>

#include "structs/user.h"
#include "chatingwindow.h"

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

    virtual void closeEvent(QCloseEvent *event);

public slots:
    // all in main thread
    void closeApp();
    void askClostApp();
    void aboutQt();
    void openChatWindow(QListWidgetItem *item);
    void chatWindowCloses(quint32 uid); // window initiative close
    void receiveMsg(quint32 from_user_id, quint32 to_user_id, const QString &msgbody);
    void toOnline(quint32 uid);
    void toOffline(quint32 uid);

signals:

private:
    Ui::MainWindow *ui;
    User *_user_self;
    QMap<uint32_t, User> *_users;
    QVector<uint32_t> *_recent;

    QMap<uint32_t, ChatingWindow*> _chatwindows;
};

class ContactWidget;

class ContactManager {
public:
    class Contact {
    public:
        Contact(uint32_t uid, const QString &name);
        ContactWidget *createWidget();

        void removeWidget(ContactWidget *w);

        void toOnline();
        void toOffline();

    private:
        bool _online;
        uint32_t _userid;
        QString _name;
        QVector<ContactWidget*> *_widgets;
    };
    // / //

    static Contact* createContact(uint32_t uid, const QString &name);
    static Contact* getContact(uint32_t uid);

private:
    static QMap<uint32_t, Contact*> _contact_map;
};

class ContactWidget : public QWidget {
public:
    ContactWidget(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ContactWidget(ContactManager::Contact *p, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    virtual ~ContactWidget();
    void toOnline();
    void toOffline();
    bool isOnline();

    uint32_t userid() const;
    void userid(uint32_t id);

    void name(const QString &name);

private:
    ContactManager::Contact *_create_from;
    bool _online;
    uint32_t _userid;
    QLabel *_icon;
    QLabel *_name;
    QHBoxLayout *_layout;
    QSpacerItem *_spacer;
};

#endif // MAINWINDOW_H
