#ifndef ROSTERMANAGER_H
#define ROSTERMANAGER_H

#include <QCryptographicHash>
#include <QDomElement>
#include "StorageManager.h"

class RosterManager
{
public:
    RosterManager(StorageManager *storageManager = 0);
    void setStorageType(QString type);

    QList<Contact> getContactsList(QString jid);
    QSet<QString> getContactGroups(QString jid, QString contactJid);
    Contact getContact(QString jid, QString contactJid);
    bool addContactToRoster(QString jid, Contact contact);
    bool deleteContactToRoster(QString jid, QString contactJid);
    bool updateGroupToContact(QString jid, QString contactJid, QSet<QString> groups);
    bool updateSubscriptionToContact(QString jid, QString contactJid, QString subscription);
    bool updateAskAttributeToContact(QString jid, QString contactJid, QString ask);
    bool updateNameToContact(QString jid, QString contactJid, QString name);
    bool updateApprovedToContact(QString jid, QString contactJid, bool approved);
    bool contactExists(QString jid, QString contactJid);

private:
    StorageManager *m_storageManager;
};

#endif // ROSTERMANAGER_H
