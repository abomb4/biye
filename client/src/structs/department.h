#ifndef DEPARTMENT_H
#define DEPARTMENT_H

#include <QString>
#include <QDateTime>

class Department {
public:
    Department() : _id(0), _parentid(0) {}
    uint32_t id() const { return this->_id; }
    void id(const uint32_t n) { this->_id = n; }

    uint32_t parent_id() const { return this->_parentid; }
    void parent_id(const uint32_t n) { this->_parentid = n; }

    const QString& name() const { return this->_name; }
    void name(const QString n) { this->_name = n; }

    const QDateTime& gmt_create() const { return this->_gmtcreate; }
    void gmt_create(const QDateTime n) { this->_gmtcreate = n; }

    const QDateTime& gmt_modify() const { return this->_gmtmodify; }
    void gmt_modify(const QDateTime n) { this->_gmtmodify = n; }

private:
    uint32_t _id;
    uint32_t _parentid;
    QString _name;
    QDateTime _gmtcreate;
    QDateTime _gmtmodify;
};


#endif // DEPARTMENT_H
