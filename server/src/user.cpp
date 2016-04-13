#include "user.h"

User::User() {
    this->_id = 0;
}
User::User(const User &u) {
    this->_id = u.id();
    this->_name = u.name();
    this->_password = u.password();
}

//  members variables g/setter //

const uint32_t User::id() const { return this->_id; }
void User::id(const uint32_t newId) { this->_id = newId;}

const string& User::name() const { return this->_name; }
void User::name(const string& newName) { this->_name = newName;}

const string &User::password() const { return this->_password; }
void User::password(const string& newPassword) { this->_password = newPassword;}
