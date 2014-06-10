#include "LocalStorage.h"

LocalStorage::LocalStorage()
{

}

QString LocalStorage::getStorageType()
{
    return "Local";
}

QString LocalStorage::getPassword(QString jid)
{
    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";
    QFile userFile(filename);

    if (!userFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(userFile.readAll());
    QString password = document.object().value("password").toString();
    userFile.close();
    return password;
}

bool LocalStorage::changePassword(QString jid, QString newPassword)
{
    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";

    QFile userFile(filename);
    if (!userFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userFile.readAll());
    QJsonObject userObject = document.object();

    userObject.insert("password", newPassword);
    document.setObject(userObject);

    // We erase the contain of the file
    userFile.resize(0);
    bool ok = userFile.write(document.toJson());
    userFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::createUser(QString jid, QString password)
{
    QString userDirPath = "accounts/";
    QDir dir(userDirPath);
    if (!dir.exists())
    {
        QDir dir;
        dir.mkdir(userDirPath);
    }

    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";
    QFile userFile(filename);

    if (!userFile.open(QIODevice::WriteOnly))
        return false;

    QJsonDocument document;

    QJsonObject userObject;
    userObject.insert("jid", QJsonValue(jid.replace("_", "@")));;
    userObject.insert("password", QJsonValue(password));
    document.setObject(userObject);

    bool ok = userFile.write(document.toJson());
    userFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::deleteUser(QString jid)
{
    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";
    return QFile::remove(filename);
}

QList<Contact> LocalStorage::getContactsList(QString jid)
{
    QString contactDirPath = "roster/" + jid.replace("@", "_") + "/";
    QDir contactDir(contactDirPath);

    QList<Contact> contactList;
    QStringList contactFileList = contactDir.entryList(QDir::Files | QDir::NoDot | QDir::NoDotAndDotDot | QDir::NoDotDot);
    for (int i = 0, c = contactFileList.count(); i < c; ++i)
    {
        QString filename = contactDir.path() + "/" + contactFileList.value(i);
        QFile contactFile(filename);

        if (!contactFile.open(QIODevice::ReadOnly))
            return QList<Contact>();

        QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
        contactList << Contact::fromJsonObject(document.object());
        contactFile.close();
    }
    return contactList;
}

bool LocalStorage::addContactToRoster(QString jid, Contact contact)
{
    QDir dir;
    dir.mkdir("roster");
    dir.setPath("roster/");
    dir.mkdir(jid.replace("@", "_"));

    QString filename = "roster/" + jid + "/" + contact.getJid().replace("@", "_") + ".qjc";

    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    // First contact
    if (contactObject.isEmpty())
    {
        contactObject = contact.toJsonObject();
    }
    else
    {
        if (!contact.getName().isEmpty())
            contactObject.insert("name", contact.getName());

        if (!contact.getGroups().isEmpty())
            contactObject.insert("groups", QJsonArray::fromStringList(QStringList::fromSet(contact.getGroups())));
    }
    document.setObject(contactObject);

    contactFile.resize(0);
    bool ok = contactFile.write(document.toJson());
    contactFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::deleteContactToRoster(QString jid, QString contactJid)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";
    return QFile::remove(filename);
}

bool LocalStorage::updateGroupToContact(QString jid, QString contactJid,
                                         QSet<QString> groups)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";

    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    contactObject.insert("groups", QJsonArray::fromStringList(QStringList::fromSet(groups)));
    document.setObject(contactObject);

    contactFile.resize(0);
    quint64 ok = contactFile.write(document.toJson());
    contactFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::updateSubscriptionToContact(QString jid, QString contactJid,
                                         QString subscription)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";

    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadWrite))
    {
        Contact contact("", false, "", contactJid.replace("_", "@"), "", subscription, QSet<QString>());
        return addContactToRoster(jid, contact);
    }

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    QString contactSubscription = contactObject.value("subscription").toString();
    if (((subscription == "from") && (contactSubscription == "to")) ||
            ((subscription == "to") && (contactSubscription == "from")))
    {
        contactObject.insert("subscription", QJsonValue(QString("both")));
    }
    else
    {
        contactObject.insert("subscription", QJsonValue(subscription));
    }
    document.setObject(contactObject);

    contactFile.resize(0);
    quint64 ok = contactFile.write(document.toJson());
    contactFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::updateAskAttributeToContact(QString jid, QString contactJid,
                                         QString ask)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";

    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadWrite))
    {
        Contact contact("", false, ask, contactJid.replace("_", "@"), "", "", QSet<QString>());
        return addContactToRoster(jid, contact);
    }

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    if ((contactObject.value("subscription").toString() != "to")
            && (contactObject.value("subscription").toString() != "both"))
    {
        contactObject.insert("ask", QJsonValue(ask));
    }
    document.setObject(contactObject);

    contactFile.resize(0);
    quint64 ok = contactFile.write(document.toJson());
    contactFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::updateNameToContact(QString jid, QString contactJid,
                                         QString name)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";

    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    contactObject.insert("name", QJsonValue(name));
    document.setObject(contactObject);

    contactFile.resize(0);
    quint64 ok = contactFile.write(document.toJson());
    contactFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::updateApprovedToContact(QString jid, QString contactJid, bool approved)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";

    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    contactObject.insert("approved", QJsonValue(approved));
    document.setObject(contactObject);

    contactFile.resize(0);
    quint64 ok = contactFile.write(document.toJson());
    contactFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::userExists(QString jid)
{
    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";
    return QFile::exists(filename);
}

bool LocalStorage::contactExists(QString jid, QString contactJid)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";
    return QFile::exists(filename);
}

Contact LocalStorage::getContact(QString jid, QString contactJid)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";

    Contact contact;
    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadOnly))
        return contact;

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    contact.setApproved(contactObject.value("approved").toBool());
    contact.setAsk(contactObject.value("ask").toString());
    contact.setJid(contactObject.value("jid").toString());
    contact.setName(contactObject.value("name").toString());
    contact.setSubscription(contactObject.value("subscription").toString());
    contact.setVersion(contactObject.value("version").toString());
    contact.setGroups(contactObject.value("groups").toVariant().toStringList().toSet());

    contactFile.close();
    return contact;
}

QString LocalStorage::getContactSubscription(QString jid, QString contactJid)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";

    Contact contact;
    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    contactFile.close();
    return contactObject.value("subscription").toString();
}

QSet<QString> LocalStorage::getContactGroups(QString jid, QString contactJid)
{
    QString contactsDirPath = "roster/" + jid.replace("@", "_") + "/";
    QString filename = contactsDirPath + contactJid.replace("@", "_") + ".qjc";

    QFile contactFile(filename);
    if (!contactFile.open(QIODevice::ReadOnly))
        return QSet<QString>();

    QJsonDocument document = QJsonDocument::fromJson(contactFile.readAll());
    QJsonObject contactObject = document.object();

    contactFile.close();
    return contactObject.value("groups").toVariant().toStringList().toSet();
}

QList<PrivacyListItem> LocalStorage::getPrivacyList(QString jid, QString privacyListName)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadWrite))
        return QList<PrivacyListItem>();

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    QJsonArray privacyList = userObject.value("privacyList").toObject().value(privacyListName).toArray();

    QList<PrivacyListItem> itemList;
    for (int i = 0, c = privacyList.count(); i < c; ++i)
    {
        QJsonObject jsonPrivacyListItem = privacyList[i].toObject();
        itemList << PrivacyListItem::fromJsonObject(jsonPrivacyListItem);
    }

    qSort(itemList.begin(), itemList.end());
    privacyListFile.close();
    return itemList;
}

bool LocalStorage::addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items)
{
//    if (privacyListName == "default")
//    {
//        /* Map the default privacy list to the block list */
//        QList<QString> blocklist;
//        foreach (PrivacyListItem item, items)
//        {
//            blocklist << item.getValue();
//        }
//        addUserBlockListItems(jid, blocklist);
//    }

    QDir dir;
    dir.mkdir("privacyList");

    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    QJsonArray privacyListItems;
    for (int i = 0, c = items.count(); i < c; ++i)
    {
        privacyListItems.append(items.value(i).toJsonObject());
    }

    QJsonObject privacyListObject = userObject.value("privacyList").toObject();
    privacyListObject.insert(privacyListName, privacyListItems);
    userObject.insert("privacyList", privacyListObject);
    document.setObject(userObject);

    privacyListFile.resize(0);
    quint64 ok = privacyListFile.write(document.toJson());
    privacyListFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::deletePrivacyList(QString jid, QString privacyListName)
{
//    if (privacyListName == "default")
//    {
//        /* Map the block list to the default privacy list */
//        QList<QString> blocklist;
//        foreach (PrivacyListItem item, getPrivacyList(jid, privacyListName))
//        {
//            blocklist << item.getValue();
//        }
//        deleteUserBlockListItems(jid, blocklist);
//    }

    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    if (userObject.value("default").toString() == privacyListName)
        userObject.insert("default", QString());

    if (userObject.value("active").toString() == privacyListName)
        userObject.insert("active", QString());

    QJsonObject privacyListObject = userObject.value("privacyList").toObject();
    privacyListObject.remove(privacyListName);

    userObject.insert("privacyList", privacyListObject);
    document.setObject(userObject);

    privacyListFile.resize(0);
    quint64 ok = privacyListFile.write(document.toJson());
    privacyListFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::privacyListExist(QString jid, QString privacyListName)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    bool listExist = userObject.value("privacyList").toObject().value(privacyListName).toArray().isEmpty();
    privacyListFile.close();
    return !listExist;
}

QString LocalStorage::getDefaultPrivacyList(QString jid)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    QString defaultList = userObject.value("default").toString();
    privacyListFile.close();

    return defaultList;
}

QString LocalStorage::getActivePrivacyList(QString jid)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    QString activeList = userObject.value("active").toString();
    privacyListFile.close();

    return activeList;
}

bool LocalStorage::setDefaultPrivacyList(QString jid, QString defaultList)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    userObject.insert("default", defaultList);
    document.setObject(userObject);

    privacyListFile.resize(0);
    quint64 ok = privacyListFile.write(document.toJson());
    privacyListFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::setActivePrivacyList(QString jid, QString activeList)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    userObject.insert("active", activeList);
    document.setObject(userObject);

    privacyListFile.resize(0);
    quint64 ok = privacyListFile.write(document.toJson());
    privacyListFile.close();
    return (true ? (ok >= 0) : false);
}

QList<PrivacyListItem> LocalStorage::getPrivacyListDenyItems(QString jid, QString privacyListName,
                                                              QString stanzaType)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadOnly))
        return QList<PrivacyListItem>();

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    QJsonArray privacyList = userObject.value("privacyList").toObject().value(privacyListName).toArray();

    QList<PrivacyListItem> itemList;
    for (int i = 0, c = privacyList.count(); i < c; ++i)
    {
        QStringList childs = privacyList[i].toObject().value("childs").toVariant().toStringList();
        if (childs.contains(stanzaType) || childs.isEmpty())
            itemList << PrivacyListItem::fromJsonObject(privacyList[i].toObject());
    }

    privacyListFile.close();
    return itemList;
}

QStringList LocalStorage::getPrivacyListNames(QString jid)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadWrite))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();

    QStringList privacyListNames = userObject.value("privacyList").toObject().toVariantMap().keys();
    privacyListFile.close();

    return privacyListNames;
}

QString LocalStorage::getVCard(QString jid)
{
    QString filename = "vCard/" + jid.replace("@", "_") + ".qjv";

    QFile vCardFile(filename);
    if (!vCardFile.open(QIODevice::ReadOnly))
        return "";

    QJsonDocument document = QJsonDocument::fromJson(vCardFile.readAll());
    QString vCard = document.object().value("vCard").toString();
    vCardFile.close();

    return vCard;
}

bool LocalStorage::updateVCard(QString jid, QString vCardInfos)
{
    QDir dir;
    dir.mkdir("vCard");

    QString filename = "vCard/" + jid.replace("@", "_") + ".qjv";

    QFile vCardFile(filename);
    if (!vCardFile.open(QIODevice::WriteOnly))
        return false;

    QJsonDocument document;

    QJsonObject vCardObject;
    vCardObject.insert("vCard", vCardInfos);
    document.setObject(vCardObject);

    quint64 ok = vCardFile.write(document.toJson());
    vCardFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::vCardExist(QString jid)
{
    QString filename = "vCard/" + jid.replace("@", "_") + ".qjv";
    return QFile::exists(filename);
}

QString LocalStorage::getLastLogoutTime(QString jid)
{
    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";

    QFile userFile(filename);
    if (!userFile.open(QIODevice::ReadWrite))
        return "";

    QJsonDocument document = QJsonDocument::fromJson(userFile.readAll());
    QString logoutTime = document.object().value("lastLogoutTime").toString();
    userFile.close();

    return logoutTime;
}

bool LocalStorage::setLastLogoutTime(QString jid, QString lastLogoutTime)
{
    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";

    QFile userFile(filename);
    if (!userFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userFile.readAll());
    QJsonObject userObject = document.object();
    userObject.insert("lastLogoutTime", lastLogoutTime);
    document.setObject(userObject);

    qDebug() << "set last logout time : " << " jid = " << jid << " time = " << lastLogoutTime << " document = " <<
                document.toJson();

    userFile.resize(0);
    quint64 ok = userFile.write(document.toJson());
    userFile.close();
    return (true ? (ok >= 0) : false);
}

QString LocalStorage::getLastStatus(QString jid)
{
    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";

    QFile userFile(filename);
    if (!userFile.open(QIODevice::ReadWrite))
        return "";

    QJsonDocument document = QJsonDocument::fromJson(userFile.readAll());
    QString lastStatus = document.object().value("lastStatus").toString();
    userFile.close();

    return lastStatus;
}

bool LocalStorage::setLastStatus(QString jid, QString status)
{
    QString filename = "accounts/" + jid.replace("@", "_") + ".qju";

    QFile userFile(filename);
    if (!userFile.open(QIODevice::ReadWrite))
        return false;

    qDebug() << "set last status : jid = " << jid << " status = " << status;

    QJsonDocument document = QJsonDocument::fromJson(userFile.readAll());
    QJsonObject userObject = document.object();
    userObject.insert("lastStatus", status);
    document.setObject(userObject);

    userFile.resize(0);
    quint64 ok = userFile.write(document.toJson());
    userFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::storePrivateData(QString jid, QMultiHash<QString, QString> nodeMap)
{
    QString privateDataDirPath = "privateData/";
    QDir dir(privateDataDirPath);
    if (!dir.exists())
    {
        QDir dir;
        dir.mkdir("privateData");
    }

    QString filename = privateDataDirPath + jid.replace("@", "_") + ".qjp";
    QFile userPrivateDataFile(filename);

    if (!userPrivateDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userPrivateDataFile.readAll());
    QJsonObject jsonObject = document.object();

    QList<QString> nodeMapKeys = nodeMap.keys();
    for (int i = 0, c = nodeMapKeys.count(); i < c; ++i)
    {
        jsonObject.insert(nodeMapKeys.value(i), nodeMap.value(nodeMapKeys.value(i)));
    }
    document.setObject(jsonObject);

    userPrivateDataFile.resize(0);
    quint64 ok = userPrivateDataFile.write(document.toJson());
    userPrivateDataFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::storePrivateData(QString jid, QList<MetaContact> metaContactList)
{
    QString privateDataDirPath = "privateData/";
    QDir dir(privateDataDirPath);
    if (!dir.exists())
    {
        QDir dir;
        dir.mkdir("privateData");
    }

    QString filename = privateDataDirPath + jid.replace("@", "_") + ".qjp";
    QFile userPrivateDataFile(filename);

    if (!userPrivateDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userPrivateDataFile.readAll());
    QJsonObject jsonObject = document.object();

    QJsonArray metaContactArray;
    foreach (MetaContact metaContact, metaContactList)
    {
        metaContactArray.append(QJsonValue(metaContact.toJsonObject()));
    }

    jsonObject.insert("storage:metacontacts", metaContactArray);
    document.setObject(jsonObject);

    userPrivateDataFile.resize(0);
    quint64 ok = userPrivateDataFile.write(document.toJson());
    userPrivateDataFile.close();
    return (true ? (ok >= 0) : false);
}

QByteArray LocalStorage::getPrivateData(QString jid, QString node)
{
    QString filename = "privateData/" + jid.replace("@", "_") + ".qjp";

    QFile userPrivateDataFile(filename);
    if (!userPrivateDataFile.open(QIODevice::ReadOnly))
        return QByteArray();

    QJsonDocument document = QJsonDocument::fromJson(userPrivateDataFile.readAll());

    userPrivateDataFile.close();
    return document.object().value(node).toVariant().toByteArray();
}

QList<MetaContact> LocalStorage::getPrivateData(QString jid)
{
    QString filename = "privateData/" + jid.replace("@", "_") + ".qjp";

    QFile userPrivateDataFile(filename);
    if (!userPrivateDataFile.open(QIODevice::ReadOnly))
        return QList<MetaContact>();

    QJsonDocument document = QJsonDocument::fromJson(userPrivateDataFile.readAll());
    QJsonArray metaContactArray = document.object().value("storage:metacontacts").toArray();
    QList<MetaContact> metaContactList;
    for (int i = 0; i < metaContactArray.count(); ++i)
    {
        metaContactList << MetaContact::fromJsonObject(metaContactArray[i].toObject());
    }
    userPrivateDataFile.close();
    return metaContactList;
}

bool LocalStorage::saveOfflineMessage(QString from, QString to, QString type,
                                      QList<QPair<QString, QString> > bodyPairList, QString stamp)
{
    QDir dir;
    dir.mkdir("offlineMessage");

    QString filename = "offlineMessage/" + to.replace("@", "_") + ".qjo";
    QString indexFilename = "offlineMessage/index_" + to + ".qji";

    QFile userOfflineMessageFile(filename);
    if (!userOfflineMessageFile.open(QIODevice::ReadWrite))
        return false;

    QFile userOfflineMessageFileIndex(indexFilename);
    if (!userOfflineMessageFileIndex.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userOfflineMessageFile.readAll());
    QJsonObject object = document.object();

    QList<QString> keys = object.toVariantMap().keys();
    QString key = QString::number(keys.value(keys.count() - 1).toInt() + 1);

    QJsonObject messageObject;
    messageObject.insert("from", from);
    messageObject.insert("type", type);
    messageObject.insert("stamp", stamp);

    QJsonArray bodyArray;
    for (int i = 0; i < bodyPairList.count(); ++i)
    {
        QJsonObject bodyObject;
        bodyObject.insert("lang", bodyPairList.value(i).first);
        bodyObject.insert("msg", bodyPairList.value(i).second);
        bodyArray.append(bodyObject);
    }
    messageObject.insert("body", bodyArray);

    object.insert(key, messageObject);
    document.setObject(object);

    userOfflineMessageFile.resize(0);
    quint64 ok = userOfflineMessageFile.write(document.toJson());
    userOfflineMessageFile.close();

    // Build indexes
    QJsonDocument indexDocument = QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll());
    QJsonObject indexObject = indexDocument.object();

    indexObject.insert(stamp, QJsonValue(key.toInt()));

    QJsonArray fromJsonArray = indexObject.value(from).toArray();
    fromJsonArray.append(key.toInt());

    indexObject.insert(from, fromJsonArray);
    indexDocument.setObject(indexObject);

    userOfflineMessageFileIndex.resize(0);
    quint64 ok1 = userOfflineMessageFileIndex.write(indexDocument.toJson());
    userOfflineMessageFileIndex.close();
    return (true ? (ok >= 0 && ok1 >= 0) : false);
}

int LocalStorage::getOfflineMessagesNumber(QString jid)
{
    QString filename = "offlineMessage/" + jid.replace("@", "_") + ".qjo";
    QFile userOfflineMessageFile(filename);
    if (!userOfflineMessageFile.open(QIODevice::ReadOnly))
        return -1;

    QJsonDocument offlineMessageDocument = QJsonDocument::fromJson(userOfflineMessageFile.readAll());
    userOfflineMessageFile.close();
    return offlineMessageDocument.object().toVariantMap().values().count();
}

QByteArray LocalStorage::getOfflineMessage(QString jid, QString stamp)
{
    QString filename = "offlineMessage/" + jid.replace("@", "_") + ".qjo";
    QString indexFilename = "offlineMessage/index_" + jid + ".qji";

    QFile userOfflineMessageFile(filename);
    if (!userOfflineMessageFile.open(QIODevice::ReadOnly))
        return QByteArray();

    QFile userOfflineMessageFileIndex(indexFilename);
    if (!userOfflineMessageFileIndex.open(QIODevice::ReadOnly))
        return QByteArray();

    QJsonDocument offlineMessageDocument = QJsonDocument::fromJson(userOfflineMessageFile.readAll());
    QJsonDocument offlineMessageFileIndexDocument = QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll());
    QString messageNumberKey = offlineMessageFileIndexDocument.object().value(stamp).toString();

    QJsonObject messageObject = offlineMessageDocument.object().value(messageNumberKey).toObject();

    QDomDocument document;
    QDomElement messageElement = document.createElement("message");
    messageElement.setAttribute("from", messageObject.value("from").toString());
    messageElement.setAttribute("to", jid.replace("_", "@"));
    messageElement.setAttribute("type", messageObject.value("type").toString());

    QJsonArray bodyArray = messageObject.value("body").toArray();
    for (int i = 0; i < bodyArray.count(); ++i)
    {
        QDomElement bodyElement = document.createElement("body");
        bodyElement.appendChild(document.createTextNode(bodyArray[i].toObject().value("msg").toString()));
        if (!bodyArray[i].toObject().value("lang").toString().isEmpty())
            bodyElement.setAttribute("xml:lang", bodyArray[i].toObject().value("lang").toString());

        messageElement.appendChild(bodyElement);
    }
    document.appendChild(messageElement);

    userOfflineMessageFile.close();
    userOfflineMessageFileIndex.close();

    return document.toByteArray();
    //return offlineMessageDocument.object().value(messageNumberKey).toObject().value("stanza").toVariant().toByteArray();
}

QMultiHash<QString, QByteArray> LocalStorage::getOfflineMessageFrom(QString jid, QString from)
{
    QString filename = "offlineMessage/" + jid.replace("@", "_") + ".qjo";
    QString indexFilename = "offlineMessage/index_" + jid + ".qji";

    QFile userOfflineMessageFile(filename);
    if (!userOfflineMessageFile.open(QIODevice::ReadOnly))
        return QMultiHash<QString, QByteArray>();

    QFile userOfflineMessageFileIndex(indexFilename);
    if (!userOfflineMessageFileIndex.open(QIODevice::ReadOnly))
        return QMultiHash<QString, QByteArray>();

    QJsonDocument offlineMessageDocument = QJsonDocument::fromJson(userOfflineMessageFile.readAll());
    QJsonDocument offlineMessageFileIndexDocument = QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll());
    QJsonArray messageNumberKeyList = offlineMessageFileIndexDocument.object().value(from).toArray();

    QMultiHash<QString, QByteArray> offlineMessageList;
    for (int i = 0; i < messageNumberKeyList.count(); ++i)
    {
        QJsonObject messageObject = offlineMessageDocument.object().value(messageNumberKeyList[i].toString()).toObject();

        QDomDocument document;
        QDomElement messageElement = document.createElement("message");
        messageElement.setAttribute("from", messageObject.value("from").toString());
        messageElement.setAttribute("to", jid.replace("_", "@"));
        messageElement.setAttribute("type", messageObject.value("type").toString());

        QJsonArray bodyArray = messageObject.value("body").toArray();
        for (int i = 0; i < bodyArray.count(); ++i)
        {
            QDomElement bodyElement = document.createElement("body");
            bodyElement.appendChild(document.createTextNode(bodyArray[i].toObject().value("msg").toString()));
            if (!bodyArray[i].toObject().value("lang").toString().isEmpty())
                bodyElement.setAttribute("xml:lang", bodyArray[i].toObject().value("lang").toString());

            messageElement.appendChild(bodyElement);
        }
        document.appendChild(messageElement);
        offlineMessageList.insert(messageObject.value("stamp").toVariant().toString(), document.toByteArray());
    }
    userOfflineMessageFile.close();
    userOfflineMessageFileIndex.close();
    return offlineMessageList;
}

QMultiHash<QString, QByteArray> LocalStorage::getAllOfflineMessage(QString jid)
{
    QString filename = "offlineMessage/" + jid.replace("@", "_") + ".qjo";
    QFile userOfflineMessageFile(filename);
    if (!userOfflineMessageFile.open(QIODevice::ReadOnly))
        return QMultiHash<QString, QByteArray>();

    QJsonDocument offlineMessageDocument = QJsonDocument::fromJson(userOfflineMessageFile.readAll());
    QList<QVariant> offlineMessageJsonList = offlineMessageDocument.object().toVariantMap().values();

    QMultiHash<QString, QByteArray> offlineMessageList;
    foreach (QVariant offlineMessageJson, offlineMessageJsonList)
    {
        QJsonObject messageObject = offlineMessageJson.toJsonObject();

        QDomDocument document;
        QDomElement messageElement = document.createElement("message");
        messageElement.setAttribute("from", messageObject.value("from").toString());
        messageElement.setAttribute("to", jid.replace("_", "@"));
        messageElement.setAttribute("type", messageObject.value("type").toString());

        QJsonArray bodyArray = messageObject.value("body").toArray();
        for (int i = 0; i < bodyArray.count(); ++i)
        {
            QDomElement bodyElement = document.createElement("body");
            bodyElement.appendChild(document.createTextNode(bodyArray[i].toObject().value("msg").toString()));
            if (!bodyArray[i].toObject().value("lang").toString().isEmpty())
                bodyElement.setAttribute("xml:lang", bodyArray[i].toObject().value("lang").toString());

            messageElement.appendChild(bodyElement);
        }
        document.appendChild(messageElement);
        offlineMessageList.insert(messageObject.value("stamp").toString(), document.toByteArray());
    }
    userOfflineMessageFile.close();
    return offlineMessageList;
}

bool LocalStorage::deleteOfflineMessage(QString jid, QString key)
{
    QString filename = "offlineMessage/" + jid.replace("@", "_") + ".qjo";
    QString indexFilename = "offlineMessage/index_" + jid + ".qji";

    QFile userOfflineMessageFile(filename);
    if (!userOfflineMessageFile.open(QIODevice::ReadOnly))
        return false;

    QFile userOfflineMessageFileIndex(indexFilename);
    if (!userOfflineMessageFileIndex.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument offlineMessageDocument = QJsonDocument::fromJson(userOfflineMessageFile.readAll());
    QJsonObject offlineMessageObject = offlineMessageDocument.object();

    if (key.contains("@"))
    {
        QJsonDocument offlineMessageFileIndexDocument = QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll());
        QJsonArray messageNumberKeyList = offlineMessageFileIndexDocument.object().value(key).toArray();
        for (int i = 0; i < messageNumberKeyList.count(); ++i)
        {
            offlineMessageObject.remove(messageNumberKeyList[i].toString());
        }
    }
    else
    {
        QJsonDocument offlineMessageFileIndexDocument = QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll());
        offlineMessageObject.remove(offlineMessageFileIndexDocument.object().value(key).toString());
    }

    offlineMessageDocument.setObject(offlineMessageObject);
    userOfflineMessageFile.resize(0);
    quint64 ok = userOfflineMessageFile.write(offlineMessageDocument.toJson());
    userOfflineMessageFileIndex.close();
    userOfflineMessageFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::deleteAllOfflineMessage(QString jid)
{
    QString filename = "offlineMessage/" + jid.replace("@", "_") + ".qjo";
    QString indexFilename = "offlineMessage/index_" + jid + ".qji";

    QFile userOfflineMessageFile(filename);
    if (!userOfflineMessageFile.open(QIODevice::ReadOnly))
        return false;

    QFile userOfflineMessageFileIndex(indexFilename);
    if (!userOfflineMessageFileIndex.open(QIODevice::ReadOnly))
        return false;

    bool ok = userOfflineMessageFile.resize(0);
    bool ok1 = userOfflineMessageFileIndex.resize(0);
    userOfflineMessageFile.close();
    userOfflineMessageFileIndex.close();
    return (ok && ok1);
}

QMultiHash<QString, QString> LocalStorage::getOfflineMessageHeaders(QString jid)
{
    QString filename = "offlineMessage/" + jid.replace("@", "_") + ".qjo";
    QFile userOfflineMessageFile(filename);
    if (!userOfflineMessageFile.open(QIODevice::ReadOnly))
        return QMultiHash<QString, QString>();

    QJsonDocument offlineMessageDocument = QJsonDocument::fromJson(userOfflineMessageFile.readAll());
    QList<QVariant> offlineMessageJsonList = offlineMessageDocument.object().toVariantMap().values();

    QMultiHash<QString, QString> offlineMessageHeaders;
    foreach (QVariant offlineMessageJson, offlineMessageJsonList)
    {
        offlineMessageHeaders.insert(offlineMessageJson.toJsonObject().value("stamp").toString(),
                                     offlineMessageJson.toJsonObject().value("from").toString());
    }
    userOfflineMessageFile.close();
    return offlineMessageHeaders;
}

bool LocalStorage::saveOfflinePresenceSubscription(QString from, QString to, QByteArray presence,
                                                   QString presenceType)
{
    qDebug() << "save offline presence subscribe";
    QDir dir;
    dir.mkdir("offlinePresenceSubscription");

    dir.setPath("offlinePresenceSubscription/");
    dir.mkdir("subscribe");
    dir.mkdir("subscribed");
    dir.mkdir("unsubscribe");
    dir.mkdir("unsubscribed");

    QString filename = "offlinePresenceSubscription/";
    if (presenceType == "subscribe")
        filename += "subscribe/";
    if (presenceType == "subscribed")
        filename += "subscribed/";
    if (presenceType == "unsubscribe")
        filename += "unsubscribe/";
    if (presenceType == "unsubscribed")
        filename += "unsubscribed/";

    filename += to.replace("@", "_") + ".qjo";

    QFile userOfflinePresenceSubscriptionFile(filename);
    if (!userOfflinePresenceSubscriptionFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userOfflinePresenceSubscriptionFile.readAll());
    QJsonObject object = document.object();

    QString key = from + "," + to.replace("_", "@");
    object.insert(key, QString(presence));

    document.setObject(object);

    userOfflinePresenceSubscriptionFile.resize(0);
    quint64 ok = userOfflinePresenceSubscriptionFile.write(document.toJson());
    userOfflinePresenceSubscriptionFile.close();
    return (true ? (ok >= 0) : false);
}

QList<QVariant> LocalStorage::getOfflinePresenceSubscription(QString jid)
{
    QString filename1 = "offlinePresenceSubscription/subscribe/" + jid.replace("@", "_") + ".qjo";
    QString filename2 = "offlinePresenceSubscription/subscribed/" + jid.replace("@", "_") + ".qjo";
    QString filename3 = "offlinePresenceSubscription/unsubscribe/" + jid.replace("@", "_") + ".qjo";
    QString filename4 = "offlinePresenceSubscription/unsubscribed/" + jid.replace("@", "_") + ".qjo";

    QFile subscribeFile(filename1);
    QJsonDocument subscribeDocument;
    if (subscribeFile.open(QIODevice::ReadOnly))
        subscribeDocument = QJsonDocument::fromJson(subscribeFile.readAll());

    QFile subscribedFile(filename2);
    QJsonDocument subscribedDocument;
    if (subscribedFile.open(QIODevice::ReadOnly))
        subscribedDocument = QJsonDocument::fromJson(subscribeFile.readAll());

    QFile unsubscribeFile(filename3);
    QJsonDocument unsubscribeDocument;
    if (unsubscribeFile.open(QIODevice::ReadOnly))
        unsubscribeDocument = QJsonDocument::fromJson(subscribeFile.readAll());

    QFile unsubscribedFile(filename4);
    QJsonDocument unsubscribedDocument;
    if (unsubscribedFile.open(QIODevice::ReadOnly))
        unsubscribedDocument = QJsonDocument::fromJson(subscribeFile.readAll());

    QList<QVariant> subscriptionList;
    subscriptionList << subscribeDocument.object().toVariantMap().values()
                        << subscribedDocument.object().toVariantMap().values()
                           << unsubscribeDocument.object().toVariantMap().values()
                              << unsubscribedDocument.object().toVariantMap().values();

    subscribedFile.resize(0);
    unsubscribedFile.resize(0);
    unsubscribeFile.resize(0);

    subscribeFile.close();
    subscribedFile.close();
    unsubscribeFile.close();
    unsubscribedFile.close();

    return subscriptionList;
}

bool LocalStorage::deleteOfflinePresenceSubscribe(QString from, QString to)
{
    QString filename = "offlinePresenceSubscription/subscribe/" + to.replace("@", "_") + ".qjo";

    QFile subscribeFile(filename);
    if (!subscribeFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(subscribeFile.readAll());
    QJsonObject object = document.object();

    QString key = from + "," + to.replace("_", "@");
    object.remove(key);

    document.setObject(object);

    subscribeFile.resize(0);
    quint64 ok = subscribeFile.write(document.toJson());
    subscribeFile.close();
    return (true ? (ok >= 0) : false);
}

QList<QString> LocalStorage::getUserBlockList(QString jid)
{
    QString filename = "blocklist/" + jid.replace("@", "_") + ".qjb";

    QFile blocklistFile(filename);
    if (!blocklistFile.open(QIODevice::ReadOnly))
        return QList<QString>();

    QJsonDocument document = QJsonDocument::fromJson(blocklistFile.readAll());
    QJsonArray array = document.object().value("blocklist").toArray();
    QList<QString> blocklist;

    for (int i = 0; i < array.count(); ++i)
        blocklist << array[i].toString();

    blocklistFile.close();
    return blocklist;
}

bool LocalStorage::addUserBlockListItems(QString jid, QList<QString> items)
{
    /* Map the block list to the default privacy list */
//    QList<PrivacyListItem> privacyListItems;
//    foreach (QString item, items)
//    {
//        privacyListItems << PrivacyListItem("", item, "deny", 0,QSet<QString>());
//    }
//    addItemsToPrivacyList(jid, "default", privacyListItems);

    QDir dir;
    dir.mkdir("blocklist");

    QString filename = "blocklist/" + jid.replace("@", "_") + ".qjb";

    QFile blocklistFile(filename);
    if (!blocklistFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(blocklistFile.readAll());
    QJsonObject object = document.object();

    QJsonArray blocklistArray = object.value("blocklist").toArray();

    foreach (QString item, items)
    {
        blocklistArray.append(item);
    }

    object.insert("blocklist", blocklistArray);
    document.setObject(object);

    blocklistFile.resize(0);
    quint64 ok = blocklistFile.write(document.toJson());
    blocklistFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::deleteUserBlockListItems(QString jid, QList<QString> items)
{
    QString filename = "blocklist/" + jid.replace("@", "_") + ".qjb";

    QFile blocklistFile(filename);
    if (!blocklistFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(blocklistFile.readAll());
    QJsonObject object = document.object();

    QStringList blocklist = object.value("blocklist").toVariant().toStringList();

    foreach (QString item, items)
    {
        blocklist.removeOne(item);
    }

    object.insert("blocklist", QJsonArray::fromStringList(blocklist));
    document.setObject(object);

    blocklistFile.resize(0);
    quint64 ok = blocklistFile.write(document.toJson());
    blocklistFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::emptyUserBlockList(QString jid)
{
    QString filename = "blocklist/" + jid.replace("@", "_") + ".qjb";
    QFile file;
    return file.remove(filename);
}

bool LocalStorage::createRoom(QString roomName, QString ownerJid)
{
    QString roomService = Utils::getHost(roomName);

    QDir dir;
    dir.mkdir(roomService);

    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::WriteOnly))
        return false;

    QJsonDocument document;
    QJsonObject roomObject;
    roomObject.insert("roomName", roomName.replace("_", "@"));
    roomObject.insert("locked", true);

    QJsonArray ownerArray;
    ownerArray.append(ownerJid);

    QMap<QString, QVariant> initialConfiguration;
    initialConfiguration.insert("muc#roomconfig_roomname", Utils::getUsername(roomName));
    initialConfiguration.insert("muc#roomconfig_roomdesc", "");
    initialConfiguration.insert("muc#roomconfig_lang", "en");
    initialConfiguration.insert("muc#roomconfig_changesubject", false);
    initialConfiguration.insert("muc#roomconfig_allowinvites", false);
    initialConfiguration.insert("muc#roomconfig_allowpm", "anyone");
    initialConfiguration.insert("muc#roomconfig_maxusers", 10);
    initialConfiguration.insert("muc#roomconfig_presencebroadcast",
                                QVariant(QList<QString>() << "moderator" << "participant" << "visitor"));
    initialConfiguration.insert("muc#roomconfig_getmemberlist",
                                QVariant(QList<QString>() << "moderator" << "participant" << "visitor"));
    initialConfiguration.insert("muc#roomconfig_publicroom", true);
    initialConfiguration.insert("muc#roomconfig_persistentroom", false);
    initialConfiguration.insert("muc#roomconfig_moderatedroom", false);
    initialConfiguration.insert("muc#roomconfig_membersonly", false);
    initialConfiguration.insert("muc#roomconfig_passwordprotectedroom", false);
    initialConfiguration.insert("muc#roomconfig_roomsecret", "");
    initialConfiguration.insert("muc#maxhistoryfetch", 20);
    initialConfiguration.insert("muc#roomconfig_roomadmins", QVariant(QList<QString>() << ownerJid));
    initialConfiguration.insert("muc#roomconfig_roomowners", QVariant(QList<QString>() << ownerJid));

    roomObject.insert("roomConfig", QJsonObject::fromVariantMap(initialConfiguration));
    document.setObject(roomObject);

    quint64 ok = roomFile.write(document.toJson());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

QMultiHash<QString, QString> LocalStorage::getChatRoomNameList(QString roomService)
{
    QDir dir(roomService);
    QStringList roomFilenameList = dir.entryList(QDir::Files | QDir::NoDot | QDir::NoDotAndDotDot | QDir::NoDotDot);

    QMultiHash<QString, QString> map;
    foreach (QString roomFilename, roomFilenameList)
    {
        QFile roomFile(roomService + "/" + roomFilename);
        roomFile.open(QIODevice::ReadOnly);

        QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());

        if (document.object().value("roomConfig").toObject().value("muc#roomconfig_publicroom").toVariant().toBool())
        {
            QString roomName = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomname").toString();
            if (roomName.isEmpty())
                roomName = Utils::getUsername(document.object().value("roomName").toString());

            map.insert(QFileInfo(roomFilename).completeBaseName().replace("_", "@"), roomName);
        }
        roomFile.close();
    }
    return map;
}

bool LocalStorage::chatRoomExist(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    return QFile::exists(filename);
}

QStringList LocalStorage::getOccupantsMucJid(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QList<QString>();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QStringList occupantMucJid;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        occupantMucJid << occupantArray[i].toObject().value("mucJid").toString();
    }
    roomFile.close();
    return occupantMucJid;
}

bool LocalStorage::isPrivateOccupantsList(QString roomName)
{
    return false;
}

QList<Occupant> LocalStorage::getOccupants(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QList<Occupant>();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QList<Occupant> occupantsList;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        occupantsList << Occupant::fromJsonObject(occupantArray[i].toObject());
    }
    roomFile.close();
    return occupantsList;
}

QList<Occupant> LocalStorage::getOccupants(QString roomName, QString bareJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QList<Occupant>();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QList<Occupant> occupantsList;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (Utils::getBareJid(occupantArray[i].toObject().value("jid").toString()) == bareJid)
        {
            occupantsList << Occupant::fromJsonObject(occupantArray[i].toObject());
        }
    }
    roomFile.close();
    return occupantsList;
}

QString LocalStorage::getOccupantMucJid(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QString occupantMucJid;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("jid").toString() == jid)
        {
            occupantMucJid = occupantArray[i].toObject().value("mucJid").toString();
            break;
        }
    }
    roomFile.close();
    return occupantMucJid;
}

QString LocalStorage::getOccupantJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QString occupantJid;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occupantJid = occupantArray[i].toObject().value("jid").toString();
            break;
        }
    }
    roomFile.close();
    return occupantJid;
}

QString LocalStorage::getOccupantRole(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QString occupantRole;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("jid").toString() == jid)
        {
            occupantRole = occupantArray[i].toObject().value("role").toString();
            break;
        }
    }
    roomFile.close();
    return occupantRole;
}

QString LocalStorage::getOccupantRoleFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QString occupantRole;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occupantRole = occupantArray[i].toObject().value("role").toString();
            break;
        }
    }
    roomFile.close();
    return occupantRole;
}

QString LocalStorage::getOccupantAffiliation(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QString occupantAffiliation;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("jid").toString() == jid)
        {
            occupantAffiliation = occupantArray[i].toObject().value("affiliation").toString();
            break;
        }
    }
    roomFile.close();
    return occupantAffiliation;
}

QString LocalStorage::getOccupantAffiliationFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QString occupantAffiliation;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occupantAffiliation = occupantArray[i].toObject().value("affiliation").toString();
            break;
        }
    }
    roomFile.close();
    return occupantAffiliation;
}

Occupant LocalStorage::getOccupant(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return Occupant();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    Occupant occupant;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("jid").toString() == jid)
        {
            occupant = Occupant::fromJsonObject(occupantArray[i].toObject());
            break;
        }
    }
    roomFile.close();
    return occupant;
}

bool LocalStorage::addUserToRoom(QString roomName, Occupant occupant)
{
//    if (getOccupantMucJid(roomName, occupant.jid()) == occupant.mucJid())
//        return true;

    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject roomObject = document.object();

    QJsonArray occupantsArray = roomObject.value("occupants").toArray();
    occupantsArray.append(occupant.toJsonObject());

    roomObject.insert("occupants", occupantsArray);
    document.setObject(roomObject);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

QStringList LocalStorage::getRoomTypes(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QStringList roomType = document.object().value("roomTypes").toVariant().toStringList();
    roomFile.close();
    return roomType;
}

QString LocalStorage::getRoomName(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QString naturalRoomName = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomName").toString();
    roomFile.close();
    return naturalRoomName;
}

bool LocalStorage::isRegistered(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    bool registered = document.object().value("memberList").toVariant().toStringList().contains(jid);
    roomFile.close();
    return registered;
}

QStringList LocalStorage::getRoomRegisteredMembersList(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QStringList registerdMemberMist = document.object().value("memberList").toVariant().toStringList();
    roomFile.close();
    return registerdMemberMist;
}

bool LocalStorage::isBannedUser(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    bool banned = document.object().value("bannedList").toVariant().toStringList().contains(jid);
    roomFile.close();
    return banned;
}

bool LocalStorage::nicknameOccuped(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    bool occuped = false;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occuped = true;
            break;
        }
    }
    roomFile.close();
    return occuped;
}

bool LocalStorage::maxOccupantsLimit(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    int maxUsersConfig = document.object().value("roomConfig").toObject().value("muc#roomconfig_maxusers").toString().toInt();
    int numberOccupants = document.object().value("occupants").toArray().count();

    roomFile.close();
    return (numberOccupants == maxUsersConfig);
}

bool LocalStorage::isLockedRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    bool locked = document.object().value("locked").toBool();
    roomFile.close();
    return locked;
}

bool LocalStorage::isPasswordProtectedRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    bool passwordprotected = document.object().value("roomConfig").toObject().value("muc#roomconfig_passwordprotectedroom").toVariant().toBool();
    roomFile.close();
    return passwordprotected;
}

QString LocalStorage::getRoomPassword(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QString password = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomsecret").toString();
    roomFile.close();
    return password;
}

bool LocalStorage::canBroadcastPresence(QString roomName, QString occupantRole)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    bool canbroadcast = document.object().value("roomConfig").toObject()
            .value("muc#roomconfig_presencebroadcast").toVariant().toStringList().contains(occupantRole);
    roomFile.close();
    return canbroadcast;
}

bool LocalStorage::loggedDiscussion(QString roomName)
{
    return false;
}

//QByteArray LocalStorage::getMaxcharsHistory(QString roomName, int maxchar)
//{
//    QString roomService = Utils::getHost(roomName);
//    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";

//    QFile historyFile(historyFilename);
//    if (!historyFile.open(QIODevice::ReadOnly))
//        return false;

//    QJsonDocument document = QJsonDocument::fromJson(historyFile.readAll());

//    QByteArray data;
//    QList<QVariant> historyList = document.object().toVariantMap().values();
//    for (int i = 0; i < historyList.count(); ++i)
//    {
//        data.append(historyList.value(i).toByteArray());
//    }
//    return data.left(maxchar);
//}

QList<QDomDocument> LocalStorage::getMaxstanzaHistory(QString roomName, int maxstanza)
{
    QString roomService = Utils::getHost(roomName);
    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";

    QFile historyFile(historyFilename);
    if (!historyFile.open(QIODevice::ReadOnly))
        return QList<QDomDocument>();

    QJsonDocument document = QJsonDocument::fromJson(historyFile.readAll());

    QList<QVariant> historyList = document.object().toVariantMap().values();

    QList<QDomDocument> historiesDocument;
    for (int i = 0; i < maxstanza; ++i)
    {
        QDomDocument document;
        document.setContent(historyList.value(i).toByteArray());
        historiesDocument << document;
    }
    return historiesDocument;
}

QList<QDomDocument> LocalStorage::getLastsecondsHistory(QString roomName, int seconds)
{
    QString roomService = Utils::getHost(roomName);

    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";
    QString historyIndexFilename = roomService + "/loggedMessage/" + roomName + ".qji";

    QFile historyFile(historyFilename);
    if (!historyFile.open(QIODevice::ReadOnly))
        return QList<QDomDocument>();

    QFile historyIndexFile(historyIndexFilename);
    if (!historyIndexFile.open(QIODevice::ReadOnly))
        return QList<QDomDocument>();

    QJsonDocument historyDocument = QJsonDocument::fromJson(historyFile.readAll());
    QJsonDocument historyIndexDocument = QJsonDocument::fromJson(historyIndexFile.readAll());

    QDateTime leftDateTime = QDateTime::currentDateTimeUtc().addSecs(seconds);
    QDateTime rightDateTime = QDateTime::currentDateTimeUtc();

    QList<QString> indexKeys = historyIndexDocument.object().toVariantMap().keys();

    QList<QDomDocument> historiesDocument;
    foreach (QString key, indexKeys)
    {
        QDateTime date = QDateTime::fromString(key, "yyyy-MM-ddThh:mm:ss.zzzZ");
        if ((leftDateTime <= date) && (date <= rightDateTime))
        {
            QDomDocument document;
            document.setContent(historyDocument.object().value(historyIndexDocument.object().value(key).toVariant().toByteArray()).toVariant().toByteArray());
            historiesDocument << document;
        }
    }
    historyFile.close();
    historyIndexFile.close();
    return historiesDocument;
}

QList<QDomDocument> LocalStorage::getHistorySince(QString roomName, QString since)
{
    QString roomService = Utils::getHost(roomName);

    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";
    QString historyIndexFilename = roomService + "/loggedMessage/" + roomName + ".qji";

    QFile historyFile(historyFilename);
    if (!historyFile.open(QIODevice::ReadOnly))
        return QList<QDomDocument>();

    QFile historyIndexFile(historyIndexFilename);
    if (!historyIndexFile.open(QIODevice::ReadOnly))
        return QList<QDomDocument>();

    QJsonDocument historyDocument = QJsonDocument::fromJson(historyFile.readAll());
    QJsonDocument historyIndexDocument = QJsonDocument::fromJson(historyIndexFile.readAll());

    QList<QString> indexKeys = historyIndexDocument.object().toVariantMap().keys();

    QList<QDomDocument> historiesDocument;

    QDateTime sinceDate = QDateTime::fromString(since, "yyyy-MM-ddThh:mm:ss.zzzZ");
    foreach (QString key, indexKeys)
    {
        QDateTime date = QDateTime::fromString(key, "yyyy-MM-ddThh:mm:ss.zzzZ");
        if (date >= sinceDate)
        {
            QDomDocument document;
            document.setContent(historyDocument.object().value(historyIndexDocument.object().value(key).toVariant().toByteArray()).toVariant().toByteArray());
            historiesDocument << document;
        }
    }
    historyFile.close();
    historyIndexFile.close();
    return historiesDocument;
}

QList<QDomDocument> LocalStorage::getHistorySinceMaxstanza(QString roomName, QString since, int maxstanza)
{
    QString roomService = Utils::getHost(roomName);

    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";
    QString historyIndexFilename = roomService + "/loggedMessage/" + roomName + ".qji";

    QFile historyFile(historyFilename);
    if (!historyFile.open(QIODevice::ReadOnly))
        return QList<QDomDocument>();

    QFile historyIndexFile(historyIndexFilename);
    if (!historyIndexFile.open(QIODevice::ReadOnly))
        return QList<QDomDocument>();

    QJsonDocument historyDocument = QJsonDocument::fromJson(historyFile.readAll());
    QJsonDocument historyIndexDocument = QJsonDocument::fromJson(historyIndexFile.readAll());

    QList<QString> indexKeys = historyIndexDocument.object().toVariantMap().keys();

    QList<QDomDocument> historiesDocument;
    QDateTime sinceDate = QDateTime::fromString(since, "yyyy-MM-ddThh:mm:ss.zzzZ");
    for (int i = 0; i < maxstanza; ++i)
    {
        QString key = indexKeys.value(i);
        QDateTime date = QDateTime::fromString(key, "yyyy-MM-ddThh:mm:ss.zzzZ");
        if (date >= sinceDate)
        {
            QDomDocument document;
            document.setContent(historyDocument.object().value(historyIndexDocument.object().value(key).toVariant().toByteArray()).toVariant().toByteArray());
            historiesDocument << document;
        }
    }
    historyFile.close();
    historyIndexFile.close();
    return historiesDocument;
}

//QList<QDomDocument> LocalStorage::getHistorySinceMaxchar(QString roomName, QString since, int maxchar)
//{
//    QList<QDomDocument>();
//}

//QList<QDomDocument> LocalStorage::getHistorySinceSeconds(QString roomName, QString since, int seconds)
//{
//    QString roomService = Utils::getHost(roomName);

//    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";
//    QString historyIndexFilename = roomService + "/loggedMessage/" + roomName + ".qji";

//    QFile historyFile(historyFilename);
//    if (!historyFile.open(QIODevice::ReadOnly))
//        return QList<QDomDocument>();

//    QFile historyIndexFile(historyIndexFilename);
//    if (!historyIndexFile.open(QIODevice::ReadOnly))
//        return QList<QDomDocument>();

//    QJsonDocument historyDocument = QJsonDocument::fromJson(historyFile.readAll());
//    QJsonDocument historyIndexDocument = QJsonDocument::fromJson(historyIndexFile.readAll());

//    QDateTime leftDateTime = QDateTime::currentDateTimeUtc().addSecs(seconds);
//    QDateTime rightDateTime = QDateTime::currentDateTimeUtc();

//    QList<QString> indexKeys = historyIndexDocument.object().toVariantMap().keys();

//    QList<QDomDocument> historiesDocument;
//    QDateTime sinceDate = QDateTime::fromString(since, "yyyy-MM-ddThh:mm:ss.zzzZ");
//    foreach (QString key, indexKeys)
//    {
//        QDateTime date = QDateTime::fromString(since, "yyyy-MM-ddThh:mm:ss.zzzZ");
//        if (key.contains(since))
//        {
//            QDateTime date = QDateTime::fromString(key, "yyyy-MM-ddThh:mm:ss.zzzZ");
//            if ((leftDateTime <= date) && (date <= rightDateTime))
//            {
//                QDomDocument document;
//                document.setContent(historyDocument.object().value(historyIndexDocument.object().value(key).toVariant().toByteArray()).toVariant().toByteArray());
//                historiesDocument << document;
//            }
//        }
//    }
//    historyFile.close();
//    historyIndexFile.close();
//    return historiesDocument;
//}

QString LocalStorage::getRoomSubject(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QString subject = document.object().value("roomSubject").toString();
    roomFile.close();
    return subject;
}

bool LocalStorage::hasVoice(QString roomName, QString mucJid)
{
    QString role = getOccupantRoleFromMucJid(roomName, mucJid);
    if ((role == "none") || (role == "visitor"))
        return false;
    return true;
}

bool LocalStorage::changeRoomNickname(QString roomName, QString jid, QString nickname)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();

    QJsonArray occupantArray = object.value("occupants").toArray();

    QJsonObject occupantObject;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("jid").toString() == jid)
        {
            occupantObject = occupantArray[i].toObject();
            occupantArray.removeAt(i);
            break;
        }
    }

    occupantObject.insert("mucJid", roomName.replace("_", "@") + "/" + nickname);
    occupantArray.append(occupantObject);
    object.insert("occupants", occupantArray);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::changeRole(QString roomName, QString mucJid, QString newRole)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();

    QJsonArray occupantArray = object.value("occupants").toArray();

    QJsonObject occupantObject;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occupantObject = occupantArray[i].toObject();
            occupantArray.removeAt(i);
            break;
        }
    }

    occupantObject.insert("role", newRole);
    occupantArray.append(occupantObject);
    object.insert("occupants", occupantArray);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::registerUser(QString roomName, Occupant occupant)
{
    addUserToRoom(roomName, occupant);

    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();

    QStringList memberList = object.value("memberList").toVariant().toStringList();
    memberList << Utils::getBareJid(occupant.jid());

    object.insert("memberList", QJsonArray::fromStringList(memberList));
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::unlockRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();
    object.insert("locked", false);

    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::submitConfigForm(QString roomName, QMap<QString, QVariant> dataFormValue)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();
    object.insert("roomConfig", QJsonObject::fromVariantMap(dataFormValue));

    QStringList roomTypes = object.value("roomTypes").toVariant().toStringList();
    if (dataFormValue.value("muc#roomconfig_membersonly").toBool())
    {
        roomTypes << "membersonly";
        object.insert("roomTypes", QJsonArray::fromStringList(roomTypes));
    }
    if (dataFormValue.value("muc#roomconfig_moderatedroom").toBool())
    {
        roomTypes << "moderatedroom";
        object.insert("roomTypes", QJsonArray::fromStringList(roomTypes));
    }
    if (dataFormValue.value("muc#roomconfig_passwordprotectedroom").toBool())
    {
        roomTypes << "passwordprotectedroom";
        object.insert("roomTypes", QJsonArray::fromStringList(roomTypes));
    }

    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    return (true ? (ok >= 0) : false);
}

QStringList LocalStorage::getRoomOwnersList(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QList<QString>();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QStringList ownersList = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomowners").toVariant().toStringList();
    roomFile.close();
    return ownersList;
}

QMap<QString, QVariant> LocalStorage::getRoomConfig(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QMap<QString, QVariant>();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QMap<QString, QVariant> roomConfig = document.object().value("roomConfig").toVariant().toMap();
    roomFile.close();
    return roomConfig;
}

bool LocalStorage::destroyRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile;
    return roomFile.remove(filename);
}

QStringList LocalStorage::getRoomModeratorsJid(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QStringList moderatorList;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("role").toString() == "moderator")
            moderatorList << occupantArray[i].toObject().value("role").toString();
    }
    roomFile.close();
    return moderatorList;
}

bool LocalStorage::removeOccupant(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();
    QJsonArray occupantArray = object.value("occupants").toArray();
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occupantArray.removeAt(i);
            break;
        }
    }

    object.insert("occupants", occupantArray);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::removeOccupants(QString roomName, QString bareJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();

    QJsonArray occupantArray = object.value("occupants").toArray();
    QList<int> index;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (Utils::getBareJid(occupantArray[i].toObject().value("jid").toString()) == bareJid)
            index << i;
    }

    for (int i = 0; i < index.count(); ++i)
        occupantArray.removeAt(index[i]);

    object.insert("occupants", occupantArray);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::changeRoomSubject(QString roomName, QString subject)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();
    object.insert("roomSubject", subject);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::canChangeRoomSubject(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    bool canChangeRoomSubject = document.object().value("roomConfig").toObject().value("muc#roomconfig_changesubject").toVariant().toBool();
    roomFile.close();
    return canChangeRoomSubject;
}

QStringList LocalStorage::getRoomAdminsList(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QStringList adminList = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomadmins").toVariant().toStringList();
    roomFile.close();
    return adminList;
}

bool LocalStorage::changeAffiliation(QString roomName, QString jid, QString newAffiliation)
{
    QStringList roomTypes = getRoomTypes(roomName);
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();

    QJsonArray occupantArray = object.value("occupants").toArray();

    QList<int> occupantIndex;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if ((Utils::getBareJid(occupantArray[i].toObject().value("jid").toString()) == jid) ||
                (occupantArray[i].toObject().value("jid").toString() == jid))
            occupantIndex << i;
    }

    QList<QJsonObject> occupantObjectList;
    for (int i = 0; i < occupantIndex.count(); ++i)
    {
        occupantObjectList << occupantArray[i].toObject();
        occupantArray.removeAt(i);
    }

    QString oldAffiliation;
    foreach (QJsonObject occupantObject, occupantObjectList)
    {
        oldAffiliation = occupantObject.value("affiliation").toString();
        occupantObject.insert("affiliation", newAffiliation);
        occupantArray.append(occupantObject);
    }
    object.insert("occupants", occupantArray);

    // add user to the admin list
    if (newAffiliation == "admin")
    {
        QJsonObject roomConfigObject = object.value("roomConfig").toObject();
        QJsonArray adminList = roomConfigObject.value("muc#roomconfig_roomadmins").toArray();

        if (!adminList.contains(Utils::getBareJid(jid)))
            adminList.append(Utils::getBareJid(jid));

        roomConfigObject.insert("muc#roomconfig_roomadmins", adminList);
        object.insert("roomConfig", roomConfigObject);
    }
    else if ((newAffiliation == "member") && roomTypes.contains("membersonly"))
    {
        QStringList memberList = object.value("memberList").toVariant().toStringList();

        if (!memberList.contains(Utils::getBareJid(jid)))
            memberList << Utils::getBareJid(jid);

        object.insert("memberList", QJsonArray::fromStringList(memberList));
    }
    else if (newAffiliation == "owner")
    {
        QJsonObject roomConfigObject = object.value("roomConfig").toObject();
        QStringList ownerList = roomConfigObject.value("muc#roomconfig_roomowners").toVariant().toStringList();

        if (!ownerList.contains((Utils::getBareJid(jid))))
            ownerList << Utils::getBareJid(jid);

        roomConfigObject.insert("muc#roomconfig_roomowners", QJsonArray::fromStringList(ownerList));
        object.insert("roomConfig", roomConfigObject);
    }
    else if ((newAffiliation == "none") || (newAffiliation == "outcast"))
    {
        if (newAffiliation == "outcast")
        {
            QStringList bannedList = object.value("bannedList").toVariant().toStringList();
            bannedList << jid;
            object.insert("bannedList", QJsonArray::fromStringList(bannedList));
        }

        if (oldAffiliation == "owner")
        {
            QJsonObject roomConfigObject = object.value("roomConfig").toObject();
            QStringList ownerList = roomConfigObject.value("muc#roomconfig_roomowners").toVariant().toStringList();
            ownerList.removeOne(Utils::getBareJid(jid));

            roomConfigObject.insert("muc#roomconfig_roomowners", QJsonArray::fromStringList(ownerList));
            object.insert("roomConfig", roomConfigObject);
        }
        else if (oldAffiliation == "admin")
        {
            QJsonObject roomConfigObject = object.value("roomConfig").toObject();
            QStringList ownerList = roomConfigObject.value("muc#roomconfig_roomadmins").toVariant().toStringList();
            ownerList.removeOne(Utils::getBareJid(jid));

            roomConfigObject.insert("muc#roomconfig_roomadmins", QJsonArray::fromStringList(ownerList));
            object.insert("roomConfig", roomConfigObject);
        }
        else if ((oldAffiliation == "member") && roomTypes.contains("membersonly"))
        {
            QStringList memberList = object.value("memberList").toVariant().toStringList();
            memberList.removeOne(Utils::getBareJid(jid));

            object.insert("memberList", QJsonArray::fromStringList(memberList));
        }
    }

    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::isPersistentRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    bool isPersistent = document.object().value("roomConfig").toObject().value("muc#roomconfig_persistentroom").toVariant().toBool();
    roomFile.close();
    return isPersistent;
}

bool LocalStorage::changeOccupantStatus(QString roomName, QString mucJid, QString status)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();
    QJsonArray occupantArray = object.value("occupants").toArray();

    QJsonObject occupantObject;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occupantObject = occupantArray[i].toObject();
            occupantArray.removeAt(i);
            break;
        }
    }
    occupantObject.insert("status", status);
    occupantArray.append(occupantObject);
    object.insert("occupants", occupantArray);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::changeOccupantShow(QString roomName, QString mucJid, QString show)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();
    QJsonArray occupantArray = object.value("occupants").toArray();

    QJsonObject occupantObject;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occupantObject = occupantArray[i].toObject();
            occupantArray.removeAt(i);
            break;
        }
    }
    occupantObject.insert("show", show);
    occupantArray.append(occupantObject);
    object.insert("occupants", occupantArray);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toJson());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

QString LocalStorage::getOccupantStatusFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();
    QJsonArray occupantArray = object.value("occupants").toArray();

    QString status;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            status = occupantArray[i].toObject().value("status").toString();
            break;
        }
    }
    roomFile.close();
    return status;
}

QString LocalStorage::getOccupantShowFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonObject object = document.object();
    QJsonArray occupantArray = object.value("occupants").toArray();

    QString show;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            show = occupantArray[i].toObject().value("show").toString();
            break;
        }
    }
    roomFile.close();
    return show;
}

Occupant LocalStorage::getOccupantFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return Occupant();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    Occupant occupant;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("mucJid").toString() == mucJid)
        {
            occupant = Occupant::fromJsonObject(occupantArray[i].toObject());
            break;
        }
    }
    roomFile.close();
    return occupant;
}

bool LocalStorage::saveMucMessage(QString roomName, QByteArray message, QString stamp)
{
    QString roomService = Utils::getHost(roomName);
    QDir dir(roomService);
    dir.mkdir("loggedMessage");

    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";
    QString historyIndexFilename = roomService + "/loggedMessage/" + roomName + ".qji";

    QFile historyFile(historyFilename);
    if (!historyFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument historyDocument = QJsonDocument::fromJson(historyFile.readAll());
    QJsonObject object = historyDocument.object();

    QList<QString> keys = object.toVariantMap().keys();
    QString key = QString::number(keys.value(keys.count() - 1).toInt() + 1);

    object.insert(key, QString(message));
    historyDocument.setObject(object);

    historyFile.resize(0);
    bool ok = historyFile.write(historyDocument.toJson());
    historyFile.close();

    // Build index
    QFile historyIndexFile(historyIndexFilename);
    if (!historyIndexFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument historyIndexDocument = QJsonDocument::fromJson(historyIndexFile.readAll());
    QJsonObject indexObject = historyIndexDocument.object();

    indexObject.insert(stamp, key);
    historyIndexDocument.setObject(indexObject);

    historyIndexFile.resize(0);
    bool ok1 = historyIndexFile.write(historyIndexDocument.toJson());
    historyIndexFile.close();

    return (ok && ok1);
}

int LocalStorage::getRoomMaxhistoryFetch(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return 0;

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    roomFile.close();


    return document.object().value("roomConfig").toObject().value("muc#maxhistoryfetch").toVariant().toInt();
}

QStringList LocalStorage::getBannedList(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromJson(roomFile.readAll());
    QStringList bannedList = document.object().value("bannedList").toVariant().toStringList();
    roomFile.close();

    return bannedList;
}

