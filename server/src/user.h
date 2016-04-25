#ifndef USER_H
#define USER_H

#include <string>
using std::string;

///
/// \brief The User class(like a struct)
///
class User
{
public:
    User() {
        this->_id = 0;
    }
    User(const User &u) {
        this->_id = u.id();
        this->_name = u.name();
        this->_password = u.password();
        this->_true_name = u.trueName();
        this->_department = u.department();
        this->_icon = u.icon();
        this->_status = u.status();
        this->_gmt_create = u.gmtCreate();
        this->_gmt_modify = u.gmtModify();
    }


    uint32_t id() const { return this->_id; }
    void id(const uint32_t n) { this->_id = n; }

    const string& name() const { return this->_name; }
    void name(const string n) { this->_name = n; }

    const string& password() const { return this->_password; }
    void password(const string n) { this->_password = n; }

    const string& trueName() const { return this->_true_name; }
    void trueName(const string n) { this->_true_name = n; }

    uint32_t department() const { return this->_department; }
    void department(const uint32_t n) { this->_department = n; }

    const string& icon() const { return this->_icon; }
    void icon(const string n) { this->_icon = n; }

    char status() const { return this->_status; }
    void status(const char n) { this->_status = n; }

    const string& gmtCreate() const { return this->_gmt_create; }
    void gmtCreate(const string n) { this->_gmt_create = n; }

    const string& gmtModify() const { return this->_gmt_modify; }
    void gmtModify(const string n) { this->_gmt_modify = n; }

private:
    uint32_t _id;
    string _name;
    string _password;
    string _true_name;
    uint32_t _department;
    string _icon;
    char _status;
    string _gmt_create;
    string _gmt_modify;

};
#endif // USER_H
