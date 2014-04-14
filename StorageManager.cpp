#include "StorageManager.h"

StorageManager::StorageManager(QString storageType)
{
    if (storageType == "local")
    {
        m_storage = new LocalStorage();
    }
    /*else if (storageType == "mysql")
    {
        // We authenticate user using mysql
        m_storage = new MySqlStorage();
    }
    else if (storageType == "pgsql")
    {
        // We authenticate user using postgresql
        m_storage = new PGSqlStorage();
    }
    else if (storageType == "nosqlmongodb")
    {
        // We authenticate user using no sql mongodb database
        m_storage = new MongoDBStorage();
    }*/
}

Storage* StorageManager::getStorage()
{
    return m_storage;
}

QString StorageManager::getStorageType()
{

}

void StorageManager::setStorageType(QString type)
{
    if (type == "local")
    {

    }
    else if (type == "mysql")
    {
        // We authenticate user using mysql
    }
    else if (type == "pgsql")
    {
        // We authenticate user using postgresql
    }
    else if (type == "nosqlmongodb")
    {
        // We authenticate user using no sql mongodb database
    }
}
