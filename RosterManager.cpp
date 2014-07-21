#include "RosterManager.h"

RosterManager::RosterManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
}

/*
 * Get the roster list of user
 */

QList<Contact> RosterManager::getContactsList(QString jid)
{
    return m_storageManager->getStorage()->getContactsList(jid);
}

bool RosterManager::addContactToRoster(QString jid, Contact contact)
{
    return m_storageManager->getStorage()->addContactToRoster(jid, contact);
}

bool RosterManager::updateGroupToContact(QString jid, QString contactJid, QSet<QString> groups)
{
    return m_storageManager->getStorage()->updateGroupToContact(jid, contactJid, groups);
}

bool RosterManager::updateSubscriptionToContact(QString jid, QString contactJid, QString subscription)
{
    return m_storageManager->getStorage()->updateSubscriptionToContact(jid, contactJid, subscription);
}

bool RosterManager::updateAskAttributeToContact(QString jid, QString contactJid, QString ask)
{
    return m_storageManager->getStorage()->updateAskAttributeToContact(jid, contactJid, ask);
}

bool RosterManager::updateNameToContact(QString jid, QString contactJid, QString name)
{
    return m_storageManager->getStorage()->updateNameToContact(jid, contactJid, name);
}

bool RosterManager::updateApprovedToContact(QString jid, QString contactJid, bool approved)
{
    return m_storageManager->getStorage()->updateApprovedToContact(jid, contactJid, approved);
}

bool RosterManager::deleteContactToRoster(QString jid, QString contactJid)
{
    return m_storageManager->getStorage()->deleteContactToRoster(jid, contactJid);
}

bool RosterManager::contactExists(QString jid, QString contactJid)
{
    return m_storageManager->getStorage()->contactExists(jid, contactJid);
}

Contact RosterManager::getContact(QString jid, QString contactJid)
{
    return m_storageManager->getStorage()->getContact(jid, contactJid);
}

QString RosterManager::getContactSubscription(QString jid, QString contactJid)
{
    return m_storageManager->getStorage()->getContactSubscription(jid, contactJid);
}

QSet<QString> RosterManager::getContactGroups(QString jid, QString contactJid)
{
    return m_storageManager->getStorage()->getContactGroups(jid, contactJid);
}
