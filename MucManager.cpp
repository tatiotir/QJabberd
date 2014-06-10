#include "MucManager.h"

MucManager::MucManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
}

bool MucManager::createRoom(QString roomName, QString ownerJid)
{
    return m_storageManager->getStorage()->createRoom(roomName, ownerJid);
}

QMultiHash<QString, QString> MucManager::getChatRoomNameList(QString roomService)
{
    return m_storageManager->getStorage()->getChatRoomNameList(roomService);
}

bool MucManager::chatRoomExist(QString roomName)
{
    return m_storageManager->getStorage()->chatRoomExist(roomName);
}

QStringList MucManager::getOccupantsMucJid(QString roomName)
{
    return m_storageManager->getStorage()->getOccupantsMucJid(roomName);
}

bool MucManager::isPrivateOccupantsList(QString roomName)
{
    return m_storageManager->getStorage()->isPrivateOccupantsList(roomName);
}

QList<Occupant> MucManager::getOccupants(QString roomName)
{
    return m_storageManager->getStorage()->getOccupants(roomName);
}

QList<Occupant> MucManager::getOccupants(QString roomName, QString bareJid)
{
    return m_storageManager->getStorage()->getOccupants(roomName, bareJid);
}

QString MucManager::getOccupantMucJid(QString roomName, QString jid)
{
    return m_storageManager->getStorage()->getOccupantMucJid(roomName, jid);
}

QString MucManager::getOccupantJid(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->getOccupantJid(roomName, mucJid);
}

QString MucManager::getOccupantRole(QString roomName, QString jid)
{
    return m_storageManager->getStorage()->getOccupantRole(roomName, jid);
}

QString MucManager::getOccupantRoleFromMucJid(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->getOccupantRoleFromMucJid(roomName, mucJid);
}

QString MucManager::getOccupantAffiliation(QString roomName, QString jid)
{
    return m_storageManager->getStorage()->getOccupantAffiliation(roomName, jid);
}

QString MucManager::getOccupantAffiliationFromMucJid(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->getOccupantAffiliationFromMucJid(roomName, mucJid);
}

Occupant MucManager::getOccupant(QString roomName, QString jid)
{
    return m_storageManager->getStorage()->getOccupant(roomName, jid);
}

Occupant MucManager::getOccupantFromMucJid(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->getOccupantFromMucJid(roomName, mucJid);
}

bool MucManager::addUserToRoom(QString roomName, Occupant occupant)
{
    return m_storageManager->getStorage()->addUserToRoom(roomName, occupant);
}

QStringList MucManager::getRoomTypes(QString roomName)
{
    return m_storageManager->getStorage()->getRoomTypes(roomName);
}

QString MucManager::getRoomName(QString roomName)
{
    return m_storageManager->getStorage()->getRoomName(roomName);
}

bool MucManager::isRegistered(QString roomName, QString jid)
{
    return m_storageManager->getStorage()->isRegistered(roomName, jid);
}

QStringList MucManager::getRoomRegisteredMembersList(QString roomName)
{
    return m_storageManager->getStorage()->getRoomRegisteredMembersList(roomName);
}

bool MucManager::isBannedUser(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->isBannedUser(roomName, mucJid);
}

bool MucManager::nicknameOccuped(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->nicknameOccuped(roomName, mucJid);
}

bool MucManager::maxOccupantsLimit(QString roomName)
{
    return m_storageManager->getStorage()->maxOccupantsLimit(roomName);
}

bool MucManager::isLockedRoom(QString roomName)
{
    return m_storageManager->getStorage()->isLockedRoom(roomName);
}

bool MucManager::isPasswordProtectedRoom(QString roomName)
{
    return m_storageManager->getStorage()->isPasswordProtectedRoom(roomName);
}

QString MucManager::getRoomPassword(QString roomName)
{
    return m_storageManager->getStorage()->getRoomPassword(roomName);
}

bool MucManager::canBroadcastPresence(QString roomName, QString occupantRole)
{
    return m_storageManager->getStorage()->canBroadcastPresence(roomName, occupantRole);
}

bool MucManager::loggedDiscussion(QString roomName)
{
    return m_storageManager->getStorage()->loggedDiscussion(roomName);
}

//QByteArray MucManager::getMaxcharsHistory(QString roomName, int maxchar)
//{
//    return m_storageManager->getStorage()->getMaxcharsHistory(roomName, maxchar);
//}

QList<QDomDocument> MucManager::getMaxstanzaHistory(QString roomName, int maxstanza)
{
    return m_storageManager->getStorage()->getMaxstanzaHistory(roomName, maxstanza);
}

QList<QDomDocument> MucManager::getLastsecondsHistory(QString roomName, int seconds)
{
    return m_storageManager->getStorage()->getLastsecondsHistory(roomName, seconds);
}

QList<QDomDocument> MucManager::getHistorySince(QString roomName, QString since)
{
    return m_storageManager->getStorage()->getHistorySince(roomName, since);
}

QList<QDomDocument> MucManager::getHistorySinceMaxstanza(QString roomName, QString since, int maxstanza)
{
    return m_storageManager->getStorage()->getHistorySinceMaxstanza(roomName, since, maxstanza);
}

//QList<QDomDocument> MucManager::getHistorySinceMaxchar(QString roomName, QString since, int maxchar)
//{
//    return m_storageManager->getStorage()->getHistorySinceMaxchar(roomName, since, maxchar);
//}

//QList<QDomDocument> MucManager::getHistorySinceSeconds(QString roomName, QString since, int seconds)
//{
//    return m_storageManager->getStorage()->getHistorySinceSeconds(roomName, since, seconds);
//}

QString MucManager::getRoomSubject(QString roomName)
{
    return m_storageManager->getStorage()->getRoomSubject(roomName);
}

bool MucManager::hasVoice(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->hasVoice(roomName, mucJid);
}

bool MucManager::changeRoomNickname(QString roomName, QString jid, QString nickname)
{
    return m_storageManager->getStorage()->changeRoomNickname(roomName, jid, nickname);
}

bool MucManager::changeRole(QString roomName, QString mucJid, QString newRole)
{
    return m_storageManager->getStorage()->changeRole(roomName, mucJid, newRole);
}

bool MucManager::registerUser(QString roomName, Occupant occupant)
{
    return m_storageManager->getStorage()->registerUser(roomName, occupant);
}

bool MucManager::unlockRoom(QString roomName)
{
    return m_storageManager->getStorage()->unlockRoom(roomName);
}

bool MucManager::submitConfigForm(QString roomName, QMultiMap<QString, QVariant> dataFormValue)
{
    return m_storageManager->getStorage()->submitConfigForm(roomName, dataFormValue);
}

QStringList MucManager::getRoomOwnersList(QString roomName)
{
    return m_storageManager->getStorage()->getRoomOwnersList(roomName);
}

QMultiMap<QString, QVariant> MucManager::getRoomConfig(QString roomName)
{
    return m_storageManager->getStorage()->getRoomConfig(roomName);
}

bool MucManager::destroyRoom(QString roomName)
{
    return m_storageManager->getStorage()->destroyRoom(roomName);
}

QStringList MucManager::getRoomModeratorsJid(QString roomName)
{
    return m_storageManager->getStorage()->getRoomModeratorsJid(roomName);
}

bool MucManager::removeOccupant(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->removeOccupant(roomName, mucJid);
}

bool MucManager::removeOccupants(QString roomName, QString bareJid)
{
    return m_storageManager->getStorage()->removeOccupants(roomName, bareJid);
}

bool MucManager::changeRoomSubject(QString roomName, QString subject)
{
    return m_storageManager->getStorage()->changeRoomSubject(roomName, subject);
}

bool MucManager::canChangeRoomSubject(QString roomName)
{
    return m_storageManager->getStorage()->canChangeRoomSubject(roomName);
}

QStringList MucManager::getRoomAdminsList(QString roomName)
{
    return m_storageManager->getStorage()->getRoomAdminsList(roomName);
}

bool MucManager::changeAffiliation(QString roomName, QString jid, QString newAffiliation)
{
    return m_storageManager->getStorage()->changeAffiliation(roomName, jid, newAffiliation);
}

bool MucManager::isPersistentRoom(QString roomName)
{
    return m_storageManager->getStorage()->isPersistentRoom(roomName);
}

bool MucManager::changeOccupantStatus(QString roomName, QString mucJid, QString status)
{
    return m_storageManager->getStorage()->changeOccupantStatus(roomName, mucJid, status);
}

bool MucManager::changeOccupantShow(QString roomName, QString mucJid, QString show)
{
    return m_storageManager->getStorage()->changeOccupantShow(roomName, mucJid, show);
}

QString MucManager::getOccupantStatusFromMucJid(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->getOccupantStatusFromMucJid(roomName, mucJid);
}

QString MucManager::getOccupantShowFromMucJid(QString roomName, QString mucJid)
{
    return m_storageManager->getStorage()->getOccupantShowFromMucJid(roomName, mucJid);
}

bool MucManager::saveMucMessage(QString roomName, QByteArray message, QString stamp)
{
    return m_storageManager->getStorage()->saveMucMessage(roomName, message, stamp);
}

int MucManager::getRoomMaxhistoryFetch(QString roomName)
{
    return m_storageManager->getStorage()->getRoomMaxhistoryFetch(roomName);
}

QStringList MucManager::getBannedList(QString roomName)
{
    return m_storageManager->getStorage()->getBannedList(roomName);
}
