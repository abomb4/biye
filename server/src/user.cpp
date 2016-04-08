#include "user.h"

User::User()
{
    this->_id = 0;
}


//  members variables g/setter //

const unsigned int User::id() const { return this->_id; }
User& User::id(const unsigned int newId) { this->_id = newId; return *this;}

const string& User::name() const { return this->_name; }
User& User::name(const string& newName) { this->_name = newName; return *this;}

const string &User::password() const { return this->_password; }
User& User::password(const string& newPassword) { this->_password = newPassword; return *this;}
