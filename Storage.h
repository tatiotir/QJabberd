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
    virtual QList<PrivacyListItem> getPrivacyListItems(QString jid, QString privacyListName, QString stanzaType,
                                                       QString action) = 0;
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
    QString getRoomSubject(QString roomName);
    QList<QDomDocument> getHistorySinceMaxstanza(QString roomName, QString since, int maxstanza);
    //QList<QDomDocument> getHistorySinceMaxchar(QString roomName, QString since, int maxchar);
    //QList<QDomDocument> getHistorySinceSeconds(QString roomName, QString since, int seconds);
    bool hasVoice(QString roomName, QString mucJid);
    bool changeRoomNickname(QString roomName, QString jid, QString nickname);
    bool changeRole(QString roomName, QString mucJid, QString newRole);
    bool registerUser(QString roomName, Occupant occupant);
    bool unlockRoom(QString roomName);
    bool submitConfigForm(QString roomName, QMap<QString, QVariant> dataFormValue);
    QStringList getRoomOwnersList(QString roomName);
    QMap<QString, QVariant> getRoomConfig(QString roomName);
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
    Occupant getOccupantFromMucJid(QString roomName, QString mucJid);
    bool saveMucMessage(QString roomName, QByteArray message, QString stamp);
    int getRoomMaxhistoryFetch(QString roomName);
    QStringList getBannedList(QString roomName);

    // Pubsub interface
    bool subscribeToNode(QString node, QString jid);
    QString nodeAccessModel(QString node);
    QString nodeOwner(QString node);
    QStringList authorizedRosterGroups(QString node);
    QStringList nodeWhiteList(QString node);
    QStringList nodeCustomerDatabase(QString node);
    QString nodeUserSubscription(QString node, QString jid);
    QString nodeUserAffiliation(QString node, QString jid);
    bool allowSubscription(QString node);
    bool nodeExist(QString node);
    bool configurationRequired(QString node);
};

#endif // STORAGE_H
