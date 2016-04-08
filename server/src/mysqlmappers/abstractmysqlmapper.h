#ifndef ABSTRACTMYSQLMAPPER_H
#define ABSTRACTMYSQLMAPPER_H

#include <mysql/mysql.h>

#include <vector>
#include <map>

using std::vector;
using std::map;

template <class t>
class AbstractMysqlMapper
{
public:
    virtual vector<t> getVector(MYSQL_RES *r) = 0;
};

#endif // ABSTRACTMYSQLMAPPER_H
