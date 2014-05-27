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
        return "";

    QString password = QJsonDocument::fromJson(userFile.readAll()).object().value("password").toString();
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
    userObject.insert("jid", QJsonValue(jid));
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

        contactList << Contact::fromJsonObject(QJsonDocument::fromJson(contactFile.readAll()).object());
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
    for (int i = 0, c = privacyList.count(); i < c; ++c)
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
    QString privacyListDirPath = "privacyList/";
    QDir dir(privacyListDirPath);
    if (!dir.exists())
    {
        QDir dir;
        dir.mkdir(privacyListDirPath);
    }

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
    userObject.value("privacyList").toObject().insert(privacyListName, privacyListItems);
    document.setObject(userObject);

    privacyListFile.resize(0);
    quint64 ok = privacyListFile.write(document.toJson());
    privacyListFile.close();
    return (true ? (ok >= 0) : false);
}

bool LocalStorage::deletePrivacyList(QString jid, QString privacyListName)
{
    QString filename = "privacyList/" + jid.replace("@", "_") + ".qjp";
    QFile privacyListFile(filename);

    if (!privacyListFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(privacyListFile.readAll());
    QJsonObject userObject = document.object();
    userObject.remove(privacyListName);
    document.setObject(userObject);

    privacyListFile.resize(0);
    quint64 ok = privacyListFile.write(document.toJson());
    privacyListFile.close();
    return (true ? (ok >= 0) : false);
}

QString LocalStorage::getVCard(QString jid)
{
    QString filename = "vCard/" + jid.replace("@", "_") + ".qjv";

    QFile vCardFile(filename);
    if (!vCardFile.open(QIODevice::ReadOnly))
        return "";

    QString vCard = QJsonDocument::fromJson(vCardFile.readAll()).object().value("vCard").toString();
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

    QString logoutTime = QJsonDocument::fromJson(userFile.readAll()).object().value("lastLogoutTime").toString();
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

    QString lastStatus = QJsonDocument::fromJson(userFile.readAll()).object().value("lastStatus").toString();
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

    userPrivateDataFile.close();

    QJsonArray metaContactArray = document.object().value("storage:metacontacts").toArray();
    QList<MetaContact> metaContactList;
    for (int i = 0; i < metaContactArray.count(); ++i)
    {
        metaContactList << MetaContact::fromJsonObject(metaContactArray[i].toObject());
    }
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
    QString messageNumberKey = QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll()).object().value(stamp).toString();

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
    QJsonArray messageNumberKeyList = QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll()).object().value(from).toArray();

    userOfflineMessageFile.close();
    userOfflineMessageFileIndex.close();

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
        QJsonArray messageNumberKeyList = QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll()).object().value(key).toArray();
        for (int i = 0; i < messageNumberKeyList.count(); ++i)
        {
            offlineMessageObject.remove(messageNumberKeyList[i].toString());
        }
    }
    else
    {
        offlineMessageObject.remove(QJsonDocument::fromJson(userOfflineMessageFileIndex.readAll()).object().value(key).toString());
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
    subscribeFile.open(QIODevice::ReadOnly);

    QFile subscribedFile(filename2);
    subscribedFile.open(QIODevice::ReadOnly);

    QFile unsubscribeFile(filename3);
    unsubscribeFile.open(QIODevice::ReadOnly);

    QFile unsubscribedFile(filename4);
    unsubscribedFile.open(QIODevice::ReadOnly);

    QJsonDocument subscribeDocument = QJsonDocument::fromJson(subscribeFile.readAll());
    QJsonDocument subscribedDocument = QJsonDocument::fromJson(subscribeFile.readAll());
    QJsonDocument unsubscribeDocument = QJsonDocument::fromJson(subscribeFile.readAll());
    QJsonDocument unsubscribedDocument = QJsonDocument::fromJson(subscribeFile.readAll());

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

//void LocalStorage::getChatRoomList(QString room)
//{

//}
