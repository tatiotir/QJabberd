#ifndef STORAGE_H
#define STORAGE_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QSet>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QString>
#include <QList>
#include "Utils.h"
#include "Contact.h"
#include "MetaContact.h"
#include "PrivacyListItem.h"

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
    //virtual QMultiHash<QString, QString> getChatRoomList(QString room) = 0;
};

#endif // STORAGE_H
