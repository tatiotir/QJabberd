#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QString>
#include "LocalStorage.h"
//#include "MysqlStorage.h"
//#include "PGSqlStorage.h"
//#include "MongoDBStorage.h"

class StorageManager
{
public:
    StorageManager(QString storageType);

    QString getStorageType();
    void setStorageType(QString type);
    Storage* getStorage();

private:
    Storage *m_storage;
};

#endif // STORAGEMANAGER_H