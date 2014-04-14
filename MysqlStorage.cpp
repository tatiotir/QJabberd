#include "mysqlstorage.h"

MySqlStorage::MySqlStorage()
{
}

QString MySqlStorage::getStorageType()
{
    return "mysql";
}

QString MySqlStorage::getPassword(QString jid)
{

}

bool MySqlStorage::changePassword(QString jid, QString newPassword)
{

}

bool MySqlStorage::createUser(QString jid, QString password)
{

}

bool MySqlStorage::deleteUser(QString jid)
{

}

QList<Contact> MySqlStorage::getContactsList(QString jid)
{

}

bool MySqlStorage::addContactToRoster(QString jid, Contact contact)
{

}

Contact *MySqlStorage::deleteContactToRoster(QString jid, QString contactJid)
{

}

bool MySqlStorage::updateGroupToContact(QString jid, QString contactJid,
                                         QSet<QString> groups)
{

}

bool MySqlStorage::updateSubscriptionToContact(QString jid, QString contactJid,
                                         QString subscription)
{

}

bool MySqlStorage::updateAskAttributeToContact(QString jid, QString contactJid,
                                         QString ask)
{

}

bool MySqlStorage::updateNameToContact(QString jid, QString contactJid,
                                         QString name)
{

}

bool MySqlStorage::userExists(QString jid)
{

}

Contact * MySqlStorage::contactExists(QString jid, QString contactJid)
{

}

Contact MySqlStorage::getContact(QString jid, QString contactJid)
{

}

QSet<QString> MySqlStorage::getContactGroups(QString jid, QString contactJid)
{

}

QList<PrivacyListItem> MySqlStorage::getPrivacyList(QString jid, QString privacyListName)
{
    return QList<PrivacyListItem>();
}

bool MySqlStorage::addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items)
{
    return true;
}

bool MySqlStorage::deletePrivacyList(QString jid, QString privacyListName)
{
    return true;
}

QByteArray MySqlStorage::getVCard(QString jid)
{

}

bool MySqlStorage::updateVCard(QString jid, QByteArray vCardInfos)
{

}

bool MySqlStorage::vCardExist(QString jid)
{

}

QString MySqlStorage::getLogoutTime(QString jid)
{

}

void MySqlStorage::setLogoutTime(QString jid, QString logoutTime)
{

}
