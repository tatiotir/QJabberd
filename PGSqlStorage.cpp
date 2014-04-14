#include "pgsqlstorage.h"

PGSqlStorage::PGSqlStorage()
{
}

QString PGSqlStorage::getStorageType()
{
    return "pgsql";
}

QString PGSqlStorage::getPassword(QString jid)
{

}

bool PGSqlStorage::changePassword(QString jid, QString newPassword)
{

}

bool PGSqlStorage::createUser(QString jid, QString password)
{

}

bool PGSqlStorage::deleteUser(QString jid)
{

}

QList<Contact> PGSqlStorage::getContactsList(QString jid)
{

}

bool PGSqlStorage::addContactToRoster(QString jid, Contact contact)
{

}

Contact *PGSqlStorage::deleteContactToRoster(QString jid, QString contactJid)
{

}

bool PGSqlStorage::updateGroupToContact(QString jid, QString contactJid,
                                         QSet<QString> groups)
{

}

bool PGSqlStorage::updateSubscriptionToContact(QString jid, QString contactJid,
                                         QString subscription)
{

}

bool PGSqlStorage::updateAskAttributeToContact(QString jid, QString contactJid,
                                         QString ask)
{

}

bool PGSqlStorage::updateNameToContact(QString jid, QString contactJid,
                                         QString name)
{

}

bool PGSqlStorage::userExists(QString jid)
{

}

Contact * PGSqlStorage::contactExists(QString jid, QString contactJid)
{

}

Contact PGSqlStorage::getContact(QString jid, QString contactJid)
{

}

QSet<QString> PGSqlStorage::getContactGroups(QString jid, QString contactJid)
{

}

QList<PrivacyListItem> PGSqlStorage::getPrivacyList(QString jid, QString privacyListName)
{
    return QList<PrivacyListItem>();
}

bool PGSqlStorage::addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items)
{
    return true;
}

bool PGSqlStorage::deletePrivacyList(QString jid, QString privacyListName)
{
    return true;
}

QByteArray PGSqlStorage::getVCard(QString jid)
{

}

bool PGSqlStorage::updateVCard(QString jid, QByteArray vCardInfos)
{

}

bool PGSqlStorage::vCardExist(QString jid)
{

}

QString PGSqlStorage::getLogoutTime(QString jid)
{

}

void PGSqlStorage::setLogoutTime(QString jid, QString logoutTime)
{

}
