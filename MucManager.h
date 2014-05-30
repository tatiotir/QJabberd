#ifndef MUCMANAGER_H
#define MUCMANAGER_H

#include <QObject>
#include "StorageManager.h"

class MucManager
{
public:
    MucManager(StorageManager *storageManager = 0);
    QMultiHash<QString, QString> getChatRoomNameList(QString roomService);
    bool chatRoomExist(QString roomName);
    QList<QString> getChatRoomOccupants(QString roomName);
    bool isPrivateOccupantsList(QString roomName);

private:
    StorageManager *m_storageManager;

};

#endif // MUCMANAGER_H
