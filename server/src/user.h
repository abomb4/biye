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
    User(const User &u);

    const uint32_t id() const;
    void id(const uint32_t newId);

    const string& name() const;
    void name(const string& newName);

    const string& password() const;
    void password(const string& newPassword);

private:
    uint32_t _id;
    string _name;
    string _password;
};

#endif // USER_H
