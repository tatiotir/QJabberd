#include "mongodbstorage.h"

MongoDBStorage::MongoDBStorage()
{
}

QString MongoDBStorage::getStorageType()
{
    return "nosqlstorage";
}

QString MongoDBStorage::getPassword(QString jid)
{

}

bool MongoDBStorage::changePassword(QString jid, QString newPassword)
{

}

bool MongoDBStorage::createUser(QString jid, QString password)
{

}

bool MongoDBStorage::deleteUser(QString jid)
{

}

QList<Contact> MongoDBStorage::getContactsList(QString jid)
{

}

bool MongoDBStorage::addContactToRoster(QString jid, Contact contact)
{

}

Contact *MongoDBStorage::deleteContactToRoster(QString jid, QString contactJid)
{

}

bool MongoDBStorage::updateGroupToContact(QString jid, QString contactJid,
                                         QSet<QString> groups)
{

}

bool MongoDBStorage::updateSubscriptionToContact(QString jid, QString contactJid,
                                         QString subscription)
{

}

bool MongoDBStorage::updateAskAttributeToContact(QString jid, QString contactJid,
                                         QString ask)
{

}

bool MongoDBStorage::updateNameToContact(QString jid, QString contactJid,
                                         QString name)
{

}

bool MongoDBStorage::userExists(QString jid)
{

}

Contact * MongoDBStorage::contactExists(QString jid, QString contactJid)
{

}

Contact MongoDBStorage::getContact(QString jid, QString contactJid)
{

}

QSet<QString> MongoDBStorage::getContactGroups(QString jid, QString contactJid)
{

}

QList<PrivacyListItem> MongoDBStorage::getPrivacyList(QString jid, QString privacyListName)
{
    return QList<PrivacyListItem>();
}

bool MongoDBStorage::addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items)
{
    return true;
}

bool MongoDBStorage::deletePrivacyList(QString jid, QString privacyListName)
{
    return true;
}

QByteArray MongoDBStorage::getVCard(QString jid)
{

}

bool MongoDBStorage::updateVCard(QString jid, QByteArray vCardInfos)
{

}

bool MongoDBStorage::vCardExist(QString jid)
{

}

QString MongoDBStorage::getLogoutTime(QString jid)
{

}

void MongoDBStorage::setLogoutTime(QString jid, QString logoutTime)
{

}
