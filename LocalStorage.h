#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <QDebug>
#include "Storage.h"

class LocalStorage : public Storage
{
public:
    LocalStorage();
    
    bool userExists(QString jid);
    bool contactExists(QString jid, QString contactJid);
    QString getStorageType();
    QString getPassword(QString jid);
    bool changePassword(QString jid, QString newPassword);
    bool createUser(QString jid, QString password);
    bool deleteUser(QString jid);
    QList<Contact> getContactsList(QString jid);
    QSet<QString> getContactGroups(QString jid, QString contactJid);
    Contact getContact(QString jid, QString contactJid);
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
    QString getVCard(QString jid);
    bool updateVCard(QString jid, QString vCardInfos);
    bool vCardExist(QString jid);
    void setLastLogoutTime(QString jid, QString lastLogoutTime);
    QString getLastLogoutTime(QString jid);
    void setLastStatus(QString jid, QString status);
    QString getLastStatus(QString jid);
    bool storePrivateData(QString jid, QMultiHash<QString, QString> nodeMap);
    bool storePrivateData(QString jid, QList<MetaContact> metaContactList);
    QByteArray getPrivateData(QString jid, QString node);
    QList<MetaContact> getPrivateData(QString jid);
    void saveOfflineMessage(QString from, QString to, QByteArray message, QString stamp);
    int getOfflineMessagesNumber(QString jid);
    QByteArray getOfflineMessage(QString jid, QString stamp);
    QMultiHash<QString, QByteArray> getOfflineMessageFrom(QString jid, QString from);
    QMultiHash<QString, QByteArray> getAllOfflineMessage(QString jid);
    QMultiHash<QString, QString> getOfflineMessageHeaders(QString jid);
    void deleteOfflineMessage(QString jid, QString key);
    void deleteAllOfflineMessage(QString jid);
    QList<QVariant> getOfflinePresenceSubscription(QString jid);
    void saveOfflinePresenceSubscription(QString from, QString to, QByteArray presence, QString presenceType);
    void deleteOfflinePresenceSubscribe(QString from, QString to);
    //QMultiHash<QString, QString> getChatRoomList(QString room);

private:
    static QString m_type;
};

#endif // LOCALSTORAGE_H
