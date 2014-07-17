#ifndef MUCMANAGER_H
#define MUCMANAGER_H

#include <QObject>
#include <QDateTime>
#include "StorageManager.h"

class MucManager
{
public:
    MucManager(StorageManager *storageManager = 0);

    bool createRoom(QString roomName, QString ownerJid);
    QMultiHash<QString, QString> getChatRoomNameList(QString roomService);
    bool chatRoomExist(QString roomName);
    QStringList getOccupantsMucJid(QString roomName);
    bool isPrivateOccupantsList(QString roomName);
    QList<Occupant> getOccupants(QString roomName);
    QList<Occupant> getOccupants(QString roomName, QString bareJid);
    QString getOccupantMucJid(QString roomName, QString jid);
    QString getOccupantJid(QString roomName, QString mucJid);
    QString getOccupantRole(QString roomName, QString jid);
    QString getOccupantRoleFromMucJid(QString roomName, QString mucJid);
    QString getOccupantAffiliation(QString roomName, QString jid);
    QString getOccupantAffiliationFromMucJid(QString roomName, QString mucJid);
    Occupant getOccupant(QString roomName, QString jid);
    Occupant getOccupantFromMucJid(QString roomName, QString mucJid);
    bool addUserToRoom(QString roomName, Occupant occupant);
    QStringList getRoomTypes(QString roomName);
    QString getRoomName(QString roomName);
    bool isRegistered(QString roomName, QString jid);
    QStringList getRoomRegisteredMembersList(QString roomName);
    bool isBannedUser(QString roomName, QString mucJid);
    bool nicknameOccuped(QString roomName, QString mucJid);
    bool maxOccupantsLimit(QString roomName);
    bool isLockedRoom(QString roomName);
    bool isPasswordProtectedRoom(QString roomName);
    QString getRoomPassword(QString roomName);
    bool canBroadcastPresence(QString roomName, QString occupantRole);
    bool loggedDiscussion(QString roomName);
    //QByteArray getMaxcharsHistory(QString roomName, int maxchar);
    QList<QDomDocument> getMaxstanzaHistory(QString roomName, int maxstanza);
    QList<QDomDocument> getLastsecondsHistory(QString roomName, int seconds);
    QList<QDomDocument> getHistorySince(QString roomName, QString since);
    QList<QDomDocument> getHistorySinceMaxstanza(QString roomName, QString since, int maxstanza);
    //QList<QDomDocument> getHistorySinceMaxchar(QString roomName, QString since, int maxchar);
    //QList<QDomDocument> getHistorySinceSeconds(QString roomName, QString since, int seconds);
    QString getRoomSubject(QString roomName);
    bool hasVoice(QString roomName, QString mucJid);
    bool changeRoomNickname(QString roomName, QString jid, QString nickname);
    bool changeRole(QString roomName, QString mucJid, QString newRole);
    bool registerUser(QString roomName, Occupant occupant);
    bool unlockRoom(QString roomName);
    bool submitConfigForm(QString roomName, QMultiMap<QString, QVariant> dataFormValue);
    QStringList getRoomOwnersList(QString roomName);
    QMultiMap<QString, QVariant> getRoomConfig(QString roomName);
    bool destroyRoom(QString roomName);
    QStringList getRoomModeratorsJid(QString roomName);
    bool removeOccupantJid(QString roomName, QString jid);
    bool removeOccupant(QString roomName, QString mucJid);
    bool removeOccupants(QString roomName, QString bareJid);
    bool changeRoomSubject(QString roomName, QString subject);
    bool canChangeRoomSubject(QString roomName);
    QStringList getRoomAdminsList(QString roomName);
    bool changeAffiliation(QString roomName, QString jid, QString newAffiliation);
    bool isPersistentRoom(QString roomName);
    bool changeOccupantStatus(QString roomName, QString mucJid, QString status);
    bool changeOccupantShow(QString roomName, QString mucJid, QString show);
    QString getOccupantStatusFromMucJid(QString roomName, QString mucJid);
    QString getOccupantShowFromMucJid(QString roomName, QString mucJid);
    bool saveMucMessage(QString roomName, QByteArray message, QString stamp);
    int getRoomMaxhistoryFetch(QString roomName);
    QStringList getBannedList(QString roomName);
    //bool allowInvitation(QString roomName);

private:
    StorageManager *m_storageManager;

};

#endif // MUCMANAGER_H
