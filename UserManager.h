#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QCryptographicHash>
#include <QDomElement>
#include "Error.h"
#include "StorageManager.h"

class UserManager
{
public:
    UserManager(StorageManager *storageManager = 0);

    void setStorageType(QString type);
    StorageManager *getStorageManager();
    QByteArray authenticate(QString id, QString username, QString password, QString resource, QString digest, QString host);
    QString getPassword(QString jid);
    bool userExists(QString jid);
    bool changePassword(QString jid, QString newPassword);
    bool createUser(QString jid, QString password);
    bool deleteUser(QString jid);

private:
    QString m_storageType;
    StorageManager *m_storageManager;
};

#endif // USERMANAGER_H
