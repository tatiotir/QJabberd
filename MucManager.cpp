#include "MucManager.h"

MucManager::MucManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
}

QMultiHash<QString, QString> MucManager::getChatRoomNameList(QString roomService)
{
    return m_storageManager->getStorage()->getChatRoomNameList(roomService);
}

bool MucManager::chatRoomExist(QString roomName)
{
    return m_storageManager->getStorage()->chatRoomExist(roomName);
}

QList<QString> MucManager::getChatRoomOccupants(QString roomName)
{
    return m_storageManager->getStorage()->getChatRoomOccupants(roomName);
}

bool MucManager::isPrivateOccupantsList(QString roomName)
{
    return m_storageManager->getStorage()->isPrivateOccupantsList(roomName);
}
