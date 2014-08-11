#ifndef MYSQLSTORAGE_H
#define MYSQLSTORAGE_H

#include "Storage.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QProcess>

class MySqlStorage : public Storage
{
public:
    MySqlStorage(QString host, int port, QString username, QString password, QString database);

    bool userExists(QString jid);
    bool contactExists(QString jid, QString contactJid);
    QString getStorageType();
    QString getPassword(QString jid);
    bool changePassword(QString jid, QString newPassword);
    bool createUser(QString jid, QString password);
    bool deleteUser(QString jid);
    QList<Contact> getContactsList(QString jid);
    QSet<QString> getContactGroups(QString jid, QString contactJid);
    QSet<QString> getGroups(QString jid);
    Contact getContact(QString jid, QString contactJid);
    QString getContactSubscription(QString jid, QString contactJid);
    bool addContactToRoster(QString jid, Contact contact);
    bool deleteContactToRoster(QString jid, QString contactJid);
    bool updateGroupToContact(QString jid, QString contactJid, QSet<QString> groups);
    //bool updateContactToRoster(QString jid, QString contactJid, QString name);
    bool updateSubscriptionToContact(QString jid, QString contactJid, QString subscription);
    bool updateAskAttributeToContact(QString jid, QString contactJid, QString ask);
    bool updateNameToContact(QString jid, QString contactJid, QString name);
    bool updateApprovedToContact(QString jid, QString contactJid, bool approved);
    QList<PrivacyListItem> getPrivacyList(QString jid, QString privacyListName);
    bool addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items);
    bool deletePrivacyList(QString jid, QString privacyListName);
    bool privacyListExist(QString jid, QString privacyListName);
    QStringList getPrivacyListNames(QString jid);
    QString getDefaultPrivacyList(QString jid);
    QString getActivePrivacyList(QString jid);
    bool setDefaultPrivacyList(QString jid, QString defaultList);
    bool setActivePrivacyList(QString jid, QString activeList);
    QList<PrivacyListItem> getPrivacyListItems(QString jid, QString privacyListName, QString stanzaType,
                                               QString action);
    QString getVCard(QString jid);
    bool updateVCard(QString jid, QString vCardInfos);
    bool vCardExist(QString jid);
    bool setLastLogoutTime(QString jid, QString lastLogoutTime);
    QString getLastLogoutTime(QString jid);
    bool setLastStatus(QString jid, QString status);
    QString getLastStatus(QString jid);
    bool storePrivateData(QString jid, QMultiHash<QString, QString> nodeMap);
    bool storePrivateData(QString jid, QList<MetaContact> metaContactList);
    QByteArray getPrivateData(QString jid, QString node);
    QList<MetaContact> getPrivateData(QString jid);
    bool saveOfflineMessage(QString from, QString to, QString type, QList<QPair<QString, QString> > bodyPairList, QString stamp);
    int getOfflineMessagesNumber(QString jid);
    QByteArray getOfflineMessage(QString jid, QString stamp);
    QMultiHash<QString, QByteArray> getOfflineMessageFrom(QString jid, QString from);
    QMultiHash<QString, QByteArray> getAllOfflineMessage(QString jid);
    QMultiHash<QString, QString> getOfflineMessageHeaders(QString jid);
    bool deleteOfflineMessage(QString jid, QString key);
    bool deleteAllOfflineMessage(QString jid);
    QList<QVariant> getOfflinePresenceSubscription(QString jid);
    bool saveOfflinePresenceSubscription(QString from, QString to, QByteArray presence, QString presenceType);
    bool deleteOfflinePresenceSubscribe(QString from, QString to);

    QList<QString> getUserBlockList(QString jid);
    bool addUserBlockListItems(QString jid, QList<QString> items);
    bool deleteUserBlockListItems(QString jid, QList<QString> items);
    bool emptyUserBlockList(QString jid);

//    bool createRoom(QString roomName, QString ownerJid);
//    QMultiHash<QString, QString> getChatRoomNameList(QString roomService);
//    bool chatRoomExist(QString roomName);
//    QStringList getOccupantsMucJid(QString roomName);
//    bool isPrivateOccupantsList(QString roomName);
//    QList<Occupant> getOccupants(QString roomName);
//    QList<Occupant> getOccupants(QString roomName, QString bareJid);
//    QString getOccupantMucJid(QString roomName, QString jid);
//    QString getOccupantJid(QString roomName, QString mucJid);
//    QString getOccupantRole(QString roomName, QString jid);
//    QString getOccupantRoleFromMucJid(QString roomName, QString mucJid);
//    QString getOccupantAffiliation(QString roomName, QString jid);
//    QString getOccupantAffiliationFromMucJid(QString roomName, QString mucJid);
//    Occupant getOccupant(QString roomName, QString jid);
//    bool addUserToRoom(QString roomName, Occupant occupant);
//    QStringList getRoomTypes(QString roomName);
//    QString getRoomName(QString roomName);
//    bool isRegistered(QString roomName, QString jid);
//    QStringList getRoomRegisteredMembersList(QString roomName);
//    bool isBannedUser(QString roomName, QString mucJid);
//    bool nicknameOccuped(QString roomName, QString mucJid);
//    bool maxOccupantsLimit(QString roomName);
//    bool isLockedRoom(QString roomName);
//    bool isPasswordProtectedRoom(QString roomName);
//    QString getRoomPassword(QString roomName);
//    bool canBroadcastPresence(QString roomName, QString occupantRole);
//    bool loggedDiscussion(QString roomName);
//    //QByteArray getMaxcharsHistory(QString roomName, int maxchar);
//    QList<QDomDocument> getMaxstanzaHistory(QString roomName, int maxstanza);
//    QList<QDomDocument> getLastsecondsHistory(QString roomName, int seconds);
//    QList<QDomDocument> getHistorySince(QString roomName, QString since);
//    QList<QDomDocument> getHistorySinceMaxstanza(QString roomName, QString since, int maxstanza);
//    //QList<QDomDocument> getHistorySinceMaxchar(QString roomName, QString since, int maxchar);
//    //QList<QDomDocument> getHistorySinceSeconds(QString roomName, QString since, int seconds);
//    QString getRoomSubject(QString roomName);
//    bool hasVoice(QString roomName, QString mucJid);
//    bool changeRoomNickname(QString roomName, QString jid, QString nickname);
//    bool changeRole(QString roomName, QString mucJid, QString newRole);
//    bool registerUser(QString roomName, Occupant occupant);
//    bool unlockRoom(QString roomName);
//    bool submitConfigForm(QString roomName, QMap<QString, QVariant> dataFormValue);
//    QStringList getRoomOwnersList(QString roomName);
//    QMap<QString, QVariant> getRoomConfig(QString roomName);
//    bool destroyRoom(QString roomName);
//    QStringList getRoomModeratorsJid(QString roomName);
//    bool removeOccupantJid(QString roomName, QString jid);
//    bool removeOccupant(QString roomName, QString mucJid);
//    bool removeOccupants(QString roomName, QString bareJid);
//    bool changeRoomSubject(QString roomName, QString subject);
//    bool canChangeRoomSubject(QString roomName);
//    QStringList getRoomAdminsList(QString roomName);
//    bool changeAffiliation(QString roomName, QString jid, QString newAffiliation);
//    bool isPersistentRoom(QString roomName);
//    bool changeOccupantStatus(QString roomName, QString mucJid, QString status);
//    bool changeOccupantShow(QString roomName, QString mucJid, QString show);
//    QString getOccupantStatusFromMucJid(QString roomName, QString mucJid);
//    QString getOccupantShowFromMucJid(QString roomName, QString mucJid);
//    Occupant getOccupantFromMucJid(QString roomName, QString mucJid);
//    bool saveMucMessage(QString roomName, QByteArray message, QString stamp);
//    int getRoomMaxhistoryFetch(QString roomName);
//    QStringList getBannedList(QString roomName);

private:
    int getUserId(QString jid);
    QSqlDatabase m_database;
    static QString m_type;
};

#endif // MYSQLSTORAGE_H
