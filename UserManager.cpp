#include "UserManager.h"

UserManager::UserManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
}

/*
 * Renvoie le password d'un utilisateur en fonction connsaissant son jabber id
 */
QString UserManager::getPassword(QString jid)
{
    return m_storageManager->getStorage()->getPassword(jid);
}

/*
 * Permet de changer le password d'un utilisateur connaissant son jabber id
 */
bool UserManager::changePassword(QString jid, QString newPassword)
{
    return m_storageManager->getStorage()->changePassword(jid, newPassword);
}

/*
 * Permet de créer un utilisateur
 */
bool UserManager::createUser(QString jid, QString password)
{
    return m_storageManager->getStorage()->createUser(jid, password);
}

/*
 * Permet de supprimer un utilisateur.
 */
bool UserManager::deleteUser(QString jid)
{
    return m_storageManager->getStorage()->deleteUser(jid);
}

/*
 * Change the storageManager storage type
 */

void UserManager::setStorageType(QString type)
{
    m_storageManager->setStorageType(type);
}

bool UserManager::userExists(QString jid)
{
    return m_storageManager->getStorage()->userExists(jid);
}

StorageManager* UserManager::getStorageManager()
{
    return m_storageManager;
}
