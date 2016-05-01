#ifndef CHATLOG_H
#define CHATLOG_H

#include <QString>
#include <QDateTime>

class ChatLog {
public:
    ChatLog() : _id(0), _sourceid(0), _targetid(0), _type(0) {}

    uint32_t id() const { return this->_id; }
    void id(const uint32_t n) { this->_id = n; }

    uint32_t source_id() const { return this->_sourceid; }
    void source_id(const uint32_t n) { this->_sourceid = n; }

    uint32_t target_id() const { return this->_targetid; }
    void target_id(const uint32_t n) { this->_targetid = n; }

    char type() const { return this->_type; }
    void type(const char n) { this->_type = n; }

    const QString& msg() const { return this->_msg; }
    void msg(const QString n) { this->_msg = n; }

    const QDateTime& gmt_create() const { return this->_gmtcreate; }
    void gmt_create(const QDateTime n) { this->_gmtcreate = n; }
private:
    uint32_t _id;
    uint32_t _sourceid;
    uint32_t _targetid;
    char _type; // 1 send 2 recieve
    QString _msg;
    QDateTime _gmtcreate;
};

#endif // CHATLOG_H
