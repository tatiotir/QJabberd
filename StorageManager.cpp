#include "StorageManager.h"

StorageManager::StorageManager(QString storageType, QJsonObject databaseSettings)
{
    if (storageType == "Local")
    {
        m_storage = new LocalStorage();
    }
    else if (storageType == "MySql")
    {
        m_storage = new MySqlStorage(databaseSettings.value("host").toString(),
                                     databaseSettings.value("port").toInt(),
                                     databaseSettings.value("username").toString(),
                                     databaseSettings.value("password").toString(),
                                     databaseSettings.value("database").toString());
    }
    else if (storageType == "PgSql")
    {
        m_storage = new PgSqlStorage(databaseSettings.value("host").toString(),
                                     databaseSettings.value("port").toInt(),
                                     databaseSettings.value("username").toString(),
                                     databaseSettings.value("password").toString(),
                                     databaseSettings.value("database").toString());
    }
    /*else if (storageType == "MongoDB")
    {
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
    if (type == "Local")
    {

    }
    else if (type == "MySql")
    {
        // We authenticate user using mysql
    }
    else if (type == "PgSql")
    {
        // We authenticate user using postgresql
    }
    else if (type == "MongoDB")
    {
        // We authenticate user using no sql mongodb database
    }
}
