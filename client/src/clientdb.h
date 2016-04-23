#ifndef CLIENTDB_H
#define CLIENTDB_H

#define DB_FILE_NAME "fxclient.dat"

class ClientDB
{
public:
    static ClientDB *getInstance();
    ~ClientDB();

private:
    ClientDB();
};

#endif // CLIENTDB_H
