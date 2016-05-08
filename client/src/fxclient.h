#ifndef FXCLIENT_H
#define FXCLIENT_H

#include <QString>
#include <QRunnable>

#include "fxmessage.h"
#include "fxconnection.h"
#include "structs/user.h"
#include "structs/department.h"
#include "structs/chatlog.h"

namespace FxChat {
class FxClient : public QObject {
    Q_OBJECT
public:
    static FxClient *getInstance();
    static void quitApplication();

    static FxChatError login(const QString *name, const QString *password);

    // update db from server, return from db
    static FxChatError getUserList(QMap<uint32_t, User> *&v);

    // return from db
    static User *getUserInfo(const uint32_t userid = USER_ID);

    // update db from server, return from db
    static FxChatError getDepartmentList(QVector<Department> *&v);

    // get from local db
    static FxChatError getRecent(QVector<uint32_t> *&v);

    // write to local db
    static FxChatError addRecent(const uint32_t target_id);

    static FxChatError getChatLog(const uint32_t target_id);

    static FxChatError sendMsg(const uint32_t to_user_id, const QString &msg);

    static bool startListenMsg();
    static bool stopListenMsg();
    static bool startHeartBeat();
    static bool stopHeartBeat();

    ~FxClient();
private:
    static FxClient *_instance;

    static uint32_t USER_ID;
    FxClient();
public slots:

signals:
    void receiveMsg(quint32 from_user_id, quint32 to_user_id, const QString &msgbody);
    void online(quint32 userid);
    void offline(quint32 userid);
};

class HeartBeatThread : public QThread {
    Q_OBJECT
public:
    HeartBeatThread() {
    }
    void run();
private:
    FxClient *_client;
};

class ListenThread : public QThread {
    Q_OBJECT
public:
    ListenThread(FxClient *c) {
        this->_client = c;
    }
    void run();
private:
    FxClient *_client;
signals:
    void recieveMsg(quint32 from_user_id, quint32 to_user_id, const QString &msgbody);
};


}

#endif // FXCLIENT_H
