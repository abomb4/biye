#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>

class User {
public:
    User() : _status(0) {}
    uint32_t id() const { return this->_id; }
    void id(const uint32_t n) { this->_id = n; }

    const QString& name() const { return this->_name; }
    void name(const QString n) { this->_name = n; }

    const QString& email() const { return this->_email; }
    void email(const QString n) { this->_email = n; }

    const QString& password() const { return this->_password; }
    void password(const QString n) { this->_password = n; }

    const QString& trueName() const { return this->_true_name; }
    void trueName(const QString n) { this->_true_name = n; }

    uint32_t department() const { return this->_department; }
    void department(const uint32_t n) { this->_department = n; }

    const QString& icon() const { return this->_icon; }
    void icon(const QString n) { this->_icon = n; }

    char status() const { return this->_status; }
    void status(const char n) { this->_status = n; }

    const QDateTime& gmtCreate() const { return this->_gmt_create; }
    void gmtCreate(const QDateTime n) { this->_gmt_create = n; }

    const QDateTime& gmtModify() const { return this->_gmt_modify; }
    void gmtModify(const QDateTime n) { this->_gmt_modify = n; }
private:
    uint32_t _id;
    QString _name;
    QString _email;
    QString _password;
    QString _true_name;
    uint32_t _department;
    QString _icon;
    char _status;
    QDateTime _gmt_create;
    QDateTime _gmt_modify;
};

#endif // USER_H
