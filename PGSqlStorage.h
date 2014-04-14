#ifndef PGSQLSTORAGE_H
#define PGSQLSTORAGE_H

#include "storage.h"

class PGSqlStorage : public Storage
{
public:
    PGSqlStorage();

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
    bool updateSubscriptionToContact(QString jid, QString contactJid, QString subscription);
    bool updateAskAttributeToContact(QString jid, QString contactJid, QString ask);
    bool updateNameToContact(QString jid, QString contactJid, QString name);
    QList<PrivacyListItem> getPrivacyList(QString jid, QString privacyListName);
    bool addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items);
    bool deletePrivacyList(QString jid, QString privacyListName);
    QByteArray getVCard(QString jid);
    bool updateVCard(QString jid, QByteArray vCardInfos);
    bool vCardExist(QString jid);
    void setLogoutTime(QString jid, QString logoutTime);
    QString getLogoutTime(QString jid);

private:
    static QString m_type;
};

#endif // PGSQLSTORAGE_H
