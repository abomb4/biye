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
    User();

    const unsigned int id() const;
    User& id(const unsigned int newId);

    const string& name() const;
    User& name(const string& newName);

    const string& password() const;
    User& password(const string& newPassword);

private:
    unsigned int _id;
    string _name;
    string _password;
};

#endif // USER_H
