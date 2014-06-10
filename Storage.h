#ifndef STORAGE_H
#define STORAGE_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QSet>
#include <QVariant>
#include <QVariantList>
#include <QFile>
#include <QDir>
#include <QString>
#include <QList>
#include "Utils.h"
#include "Contact.h"
#include "MetaContact.h"
#include "PrivacyListItem.h"
#include "Occupant.h"

class Storage
{
public:
    Storage();

    virtual bool saveOfflineMessage(QString from, QString to, QString type, QList<QPair<QString, QString> > bodyPairList, QString stamp) = 0;
    virtual int getOfflineMessagesNumber(QString jid) = 0;
    virtual QByteArray getOfflineMessage(QString jid, QString stamp) = 0;
    virtual QMultiHash<QString, QByteArray> getOfflineMessageFrom(QString jid, QString from) = 0;
    virtual QMultiHash<QString, QByteArray> getAllOfflineMessage(QString jid) = 0;
    virtual QMultiHash<QString, QString> getOfflineMessageHeaders(QString jid) = 0;
    virtual bool deleteOfflineMessage(QString jid, QString key) = 0;
    virtual bool deleteAllOfflineMessage(QString jid) = 0;
    virtual QList<QVariant> getOfflinePresenceSubscription(QString jid) = 0;
    virtual bool saveOfflinePresenceSubscription(QString from, QString to, QByteArray presence,
                                                 QString presenceType) = 0;
    virtual bool deleteOfflinePresenceSubscribe(QString from, QString to) = 0;
    bool deleteStreamData(QString smId, int h);
    bool saveStreamPresencePriority(QString smId, int presencePriority);
    bool saveStreamPresenceStanza(QString smId, QByteArray presenceData);
    bool saveStreamData(QString smId, QByteArray data);
    QList<QByteArray> getClientUnhandleStanza(QString smId);
    virtual bool userExists(QString jid) = 0;
    virtual bool contactExists(QString jid, QString contactJid) = 0;
    virtual QString getStorageType() = 0;
    virtual QString getPassword(QString jid) = 0;
    virtual bool changePassword(QString jid, QString newPassword) = 0;
    virtual bool createUser(QString jid, QString password) = 0;
    virtual bool deleteUser(QString jid) = 0;
    virtual QList<Contact> getContactsList(QString jid) = 0;
    virtual QSet<QString> getContactGroups(QString jid, QString contactJid) = 0;
    virtual Contact getContact(QString jid, QString contactJid) = 0;
    virtual QString getContactSubscription(QString jid, QString contactJid) = 0;
    virtual bool addContactToRoster(QString jid, Contact contact) = 0;
    virtual bool deleteContactToRoster(QString jid, QString contactJid) = 0;
    virtual bool updateGroupToContact(QString jid, QString contactJid, QSet<QString> groups) = 0;
    virtual bool updateSubscriptionToContact(QString jid, QString contactJid, QString subscription) = 0;
    virtual bool updateAskAttributeToContact(QString jid, QString contactJid, QString ask) = 0;
    virtual bool updateNameToContact(QString jid, QString contactJid, QString name) = 0;
    virtual bool updateApprovedToContact(QString jid, QString contactJid, bool approved) = 0;
    virtual QList<PrivacyListItem> getPrivacyList(QString jid, QString privacyListName) = 0;
    virtual bool addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items) = 0;
    virtual bool deletePrivacyList(QString jid, QString privacyListName) = 0;
    virtual QString getVCard(QString jid) = 0;
    virtual bool updateVCard(QString jid, QString vCardInfos) = 0;
    virtual bool vCardExist(QString jid) = 0;
    virtual bool setLastLogoutTime(QString jid, QString lastLogoutTime) = 0;
    virtual QString getLastLogoutTime(QString jid) = 0;
    virtual bool setLastStatus(QString jid, QString status) = 0;
    virtual QString getLastStatus(QString jid) = 0;
    virtual bool storePrivateData(QString jid, QMultiHash<QString, QString> nodeMap) = 0;
    virtual bool storePrivateData(QString jid, QList<MetaContact> metaContactList) = 0;
    virtual QByteArray getPrivateData(QString jid, QString node) = 0;
    virtual QList<MetaContact> getPrivateData(QString jid) = 0;
    virtual QList<QString> getUserBlockList(QString jid) = 0;
    virtual bool deleteUserBlockListItems(QString jid, QList<QString> items) = 0;
    virtual bool emptyUserBlockList(QString jid) = 0;
    virtual bool addUserBlockListItems(QString jid, QList<QString> items) = 0;
    virtual bool privacyListExist(QString jid, QString privacyListName) = 0;
    virtual QStringList getPrivacyListNames(QString jid) = 0;
    virtual QString getDefaultPrivacyList(QString jid) = 0;
    virtual QString getActivePrivacyList(QString jid) = 0;
    virtual bool setDefaultPrivacyList(QString jid, QString defaultList) = 0;
    virtual bool setActivePrivacyList(QString jid, QString activeList) = 0;
    virtual QList<PrivacyListItem> getPrivacyListDenyItems(QString jid, QString privacyListName, QString stanzaType) = 0;
    virtual bool createRoom(QString roomName, QString ownerJid) = 0;
    virtual QMultiHash<QString, QString> getChatRoomNameList(QString roomService) = 0;
    virtual bool chatRoomExist(QString roomName) = 0;
    virtual QStringList getOccupantsMucJid(QString roomName) = 0;
    virtual bool isPrivateOccupantsList(QString roomName) = 0;
    virtual QList<Occupant> getOccupants(QString roomName) = 0;
    virtual QList<Occupant> getOccupants(QString roomName, QString bareJid) = 0;
    virtual QString getOccupantMucJid(QString roomName, QString jid) = 0;
    virtual QString getOccupantJid(QString roomName, QString mucJid) = 0;
    virtual QString getOccupantRole(QString roomName, QString jid) = 0;
    virtual QString getOccupantRoleFromMucJid(QString roomName, QString mucJid) = 0;
    virtual QString getOccupantAffiliation(QString roomName, QString jid) = 0;
    virtual QString getOccupantAffiliationFromMucJid(QString roomName, QString mucJid) = 0;
    virtual Occupant getOccupant(QString roomName, QString jid) = 0;
    virtual bool addUserToRoom(QString roomName, Occupant occupant) = 0;
    virtual QStringList getRoomTypes(QString roomName) = 0;
    virtual QString getRoomName(QString roomName) = 0;
    virtual bool isRegistered(QString roomName, QString jid) = 0;
    virtual QStringList getRoomRegisteredMembersList(QString roomName) = 0;
    virtual bool isBannedUser(QString roomName, QString mucJid) = 0;
    virtual bool nicknameOccuped(QString roomName, QString mucJid) = 0;
    virtual bool maxOccupantsLimit(QString roomName) = 0;
    virtual bool isLockedRoom(QString roomName) = 0;
    virtual bool isPasswordProtectedRoom(QString roomName) = 0;
    virtual QString getRoomPassword(QString roomName) = 0;
    virtual bool canBroadcastPresence(QString roomName, QString occupantRole) = 0;
    virtual bool loggedDiscussion(QString roomName) = 0;
    //virtual QByteArray getMaxcharsHistory(QString roomName, int maxchar) = 0;
    virtual QList<QDomDocument> getMaxstanzaHistory(QString roomName, int maxstanza) = 0;
    virtual QList<QDomDocument> getLastsecondsHistory(QString roomName, int seconds) = 0;
    virtual QList<QDomDocument> getHistorySince(QString roomName, QString since) = 0;
    virtual QString getRoomSubject(QString roomName) = 0;
    virtual QList<QDomDocument> getHistorySinceMaxstanza(QString roomName, QString since, int maxstanza) = 0;
    //virtual QList<QDomDocument> getHistorySinceMaxchar(QString roomName, QString since, int maxchar) = 0;
    //virtual QList<QDomDocument> getHistorySinceSeconds(QString roomName, QString since, int seconds) = 0;
    virtual bool hasVoice(QString roomName, QString mucJid) = 0;
    virtual bool changeRoomNickname(QString roomName, QString jid, QString nickname) = 0;
    virtual bool changeRole(QString roomName, QString mucJid, QString newRole) = 0;
    virtual bool registerUser(QString roomName, Occupant occupant) = 0;
    virtual bool unlockRoom(QString roomName) = 0;
    virtual bool submitConfigForm(QString roomName, QMap<QString, QVariant> dataFormValue) = 0;
    virtual QStringList getRoomOwnersList(QString roomName) = 0;
    virtual QMap<QString, QVariant> getRoomConfig(QString roomName) = 0;
    virtual bool destroyRoom(QString roomName) = 0;
    virtual QStringList getRoomModeratorsJid(QString roomName) = 0;
    virtual bool removeOccupant(QString roomName, QString mucJid) = 0;
    virtual bool removeOccupants(QString roomName, QString bareJid) = 0;
    virtual bool changeRoomSubject(QString roomName, QString subject) = 0;
    virtual bool canChangeRoomSubject(QString roomName) = 0;
    virtual QStringList getRoomAdminsList(QString roomName) = 0;
    virtual bool changeAffiliation(QString roomName, QString jid, QString newAffiliation) = 0;
    virtual bool isPersistentRoom(QString roomName) = 0;
    virtual bool changeOccupantStatus(QString roomName, QString mucJid, QString status) = 0;
    virtual bool changeOccupantShow(QString roomName, QString mucJid, QString show) = 0;
    virtual QString getOccupantStatusFromMucJid(QString roomName, QString mucJid) = 0;
    virtual QString getOccupantShowFromMucJid(QString roomName, QString mucJid) = 0;
    virtual Occupant getOccupantFromMucJid(QString roomName, QString mucJid) = 0;
    virtual bool saveMucMessage(QString roomName, QByteArray message, QString stamp) = 0;
    virtual int getRoomMaxhistoryFetch(QString roomName) = 0;
    virtual QStringList getBannedList(QString roomName) = 0;
};

#endif // STORAGE_H
