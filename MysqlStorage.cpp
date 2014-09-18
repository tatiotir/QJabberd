#include "MySqlStorage.h"

MySqlStorage::MySqlStorage(QString host, int port, QString username, QString password, QString database)
{
    m_database = QSqlDatabase::addDatabase("QMYSQL");
    m_database.setHostName(host);
    m_database.setPort(port);
    m_database.setUserName(username);
    m_database.setPassword(password);
    m_database.setDatabaseName(database);
    if (!m_database.open())
    {
        qDebug() << "Database not open check database configuration";
    }
}

QString MySqlStorage::getStorageType()
{
    return "PgSql";
}

int MySqlStorage::getUserId(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();
    if (query.first())
    {
        return query.value(0).toInt();
    }
    else
    {
        return -1;
    }
}

QString MySqlStorage::getPassword(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    query.first();
    return query.value(0).toString();
}

bool MySqlStorage::changePassword(QString jid, QString newPassword)
{
    QSqlQuery query;
    query.prepare("UPDATE qjabberd_users SET password = :password WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.bindValue(":password", newPassword);
    return query.exec();
}

bool MySqlStorage::createUser(QString jid, QString password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO qjabberd_users(jid, password) VALUES(:jid, :password)");
    query.bindValue(":jid", jid);
    query.bindValue(":password", password);
    return query.exec();
}

bool MySqlStorage::deleteUser(QString jid)
{
    QSqlQuery query;
    query.prepare("DELETE FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    return query.exec();
}

QList<Contact> MySqlStorage::getContactsList(QString jid)
{
    QList<Contact> contactList;
    QSqlQuery query;
    query.prepare("SELECT version, approved, ask, jid, name, subscription, groups"
                  " FROM qjabberd_contact WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();
    while (query.next())
    {
        contactList << Contact(query.value(0).toString(), query.value(1).toBool(), query.value(2).toString(),
                               query.value(3).toString(), query.value(4).toString(), query.value(5).toString(),
                               QJsonDocument::fromJson(query.value(6).toByteArray()).object().value("groups").toVariant().toStringList().toSet());
    }
    return contactList;
}

bool MySqlStorage::addContactToRoster(QString jid, Contact contact)
{
    if (contactExists(jid, contact.getJid()))
    {
        updateNameToContact(jid, contact.getJid(), contact.getName());
        updateGroupToContact(jid, contact.getJid(), contact.getGroups());
        return true;
    }
    else
    {
        //qDebug() << "Je suis juste ici : " << getUserId(jid);
        QJsonDocument document;
        QJsonObject object;
        object.insert("groups", QJsonArray::fromStringList(QStringList::fromSet(contact.getGroups())));
        document.setObject(object);

        QSqlQuery query;
        query.prepare("INSERT INTO contact(user_id, approved, ask, groups, jid, name, subscription, version)"
                      " VALUES(:user_id, :approved, :ask, :groups, :jid, :name, :subscription, :version)");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":version", contact.getVersion());
        query.bindValue(":approved", (int)contact.getApproved());
        query.bindValue(":ask", contact.getAsk());
        query.bindValue(":jid", contact.getJid());
        query.bindValue(":name", contact.getName());
        query.bindValue(":subscription", contact.getSubscription());
        query.bindValue(":groups", document.toJson());
        return query.exec();
    }
    return false;
}

bool MySqlStorage::deleteContactToRoster(QString jid, QString contactJid)
{
    QSqlQuery query;
    query.prepare("DELETE FROM qjabberd_contact WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    return query.exec();
}

bool MySqlStorage::updateGroupToContact(QString jid, QString contactJid,
                                         QSet<QString> groups)
{
    QJsonDocument document;
    QJsonObject object;
    object.insert("groups", QJsonArray::fromStringList(QStringList::fromSet(groups)));
    document.setObject(object);

    QSqlQuery query;
    query.prepare("UPDATE qjabberd_contact SET groups = :groups WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.bindValue(":groups", document.toJson());
    return query.exec();
}

bool MySqlStorage::updateSubscriptionToContact(QString jid, QString contactJid,
                                         QString subscription)
{
    if (!contactExists(jid, contactJid))
    {
        Contact contact("", false, "", contactJid, "", subscription, QSet<QString>());
        return addContactToRoster(jid, contact);
    }

    QString contactSubscription = getContact(jid, contactJid).getSubscription();

    QSqlQuery query;
    query.prepare("UPDATE qjabberd_contact SET subscription = :subscription WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);

    if (((subscription == "from") && (contactSubscription == "to")) ||
            ((subscription == "to") && (contactSubscription == "from")))
    {
        query.bindValue(":subscription", "both");
    }
    else
    {
        query.bindValue(":subscription", subscription);
    }
    return query.exec();
}

bool MySqlStorage::updateAskAttributeToContact(QString jid, QString contactJid,
                                         QString ask)
{
    Contact contact = getContact(jid, contactJid);

    QSqlQuery query;
    query.prepare("UPDATE qjabberd_contact SET ask = :ask WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);

    if ((contact.getSubscription() != "to") && (contact.getSubscription() != "both"))
    {
        query.bindValue(":ask", ask);
    }
    return query.exec();
}

bool MySqlStorage::updateNameToContact(QString jid, QString contactJid,
                                         QString name)
{
    QSqlQuery query;
    query.prepare("UPDATE qjabberd_contact SET name = :name WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.bindValue(":name", name);
    return query.exec();
}

bool MySqlStorage::updateApprovedToContact(QString jid, QString contactJid, bool approved)
{
    QSqlQuery query;
    query.prepare("UPDATE qjabberd_contact SET approved = :approved WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.bindValue(":approved", approved);
    return query.exec();
}

bool MySqlStorage::userExists(QString jid)
{
    return (getUserId(jid) >= 0);
}

bool MySqlStorage::contactExists(QString jid, QString contactJid)
{
    int user_id = getUserId(jid);

    if (user_id == -1)
        return false;

    QSqlQuery query;
    query.prepare("SELECT id FROM qjabberd_contact WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.exec();
    return query.first();
}

Contact MySqlStorage::getContact(QString jid, QString contactJid)
{
    QSqlQuery query;
    query.prepare("SELECT version, approved, ask, jid, name, subscription, groups FROM qjabberd_contact WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.exec();

    if (query.first())
    {
        return Contact(query.value(0).toString(),
                       query.value(1).toBool(),
                       query.value(2).toString(),
                       query.value(3).toString(),
                       query.value(4).toString(),
                       query.value(5).toString(),
                       QJsonDocument::fromJson(query.value(6).toByteArray()).object().value("groups").toVariant().toStringList().toSet());
    }
    else
    {
        return Contact();
    }
}

QString MySqlStorage::getContactSubscription(QString jid, QString contactJid)
{
    QSqlQuery query;
    query.prepare("SELECT subscription FROM qjabberd_contact WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.exec();

    if (query.first())
        return query.value(0).toString();
    return QString();
}

QSet<QString> MySqlStorage::getContactGroups(QString jid, QString contactJid)
{
    QSqlQuery query;
    query.prepare("SELECT group FROM qjabberd_contact WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.exec();

    if (query.first())
        return QJsonDocument::fromJson(query.value(0).toByteArray()).object().value("groups").toVariant().toStringList().toSet();
    return QSet<QString>();
}

QSet<QString> MySqlStorage::getGroups(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT groups FROM qjabberd_contact WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    QSet<QString> groups;
    while (query.next())
    {
        foreach (QString group, QJsonDocument::fromJson(query.value(0).toByteArray()).object().value("groups").toVariant().toStringList().toSet())
            groups << group;
    }
    return groups;
}

QList<PrivacyListItem> MySqlStorage::getPrivacyList(QString jid, QString privacyListName)
{
    QSqlQuery query;
    query.prepare("SELECT type, value, action, iorder, child FROM qjabberd_privacylist WHERE user_id = :user_id AND privacyListName = :privacyListName");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":privacyListName", privacyListName);
    query.exec();

    QList<PrivacyListItem> itemList;
    while (query.next())
    {
        itemList << PrivacyListItem(query.value(0).toString(),
                                    query.value(1).toString(),
                                    query.value(2).toString(),
                                    query.value(3).toInt(),
                                    QJsonDocument::fromJson(query.value(4).toByteArray()).object().value("childs").toVariant().toStringList().toSet());
    }

    qSort(itemList.begin(), itemList.end());
    return itemList;
}

bool MySqlStorage::addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items)
{
    int user_id = getUserId(jid);

    m_database.transaction();

    QJsonDocument document;
    QJsonObject object;
    foreach (PrivacyListItem item, items)
    {
        object.insert("childs", QJsonArray::fromStringList(QStringList::fromSet(item.getChildsElement())));
        document.setObject(object);

        QSqlQuery query;
        query.prepare("INSERT INTO qjabberd_privacylist(user_id, type, value, action, iorder, child, privacyListName)"
                      " VALUES(:user_id, :type, :value, :action, :iorder, :child, :privacyListName)");
        query.bindValue(":user_id", user_id);
        query.bindValue(":type", item.getType());
        query.bindValue(":value", item.getValue());
        query.bindValue(":action", item.getAction());
        query.bindValue(":iorder", item.getOrder());
        query.bindValue(":child", document.toJson());
        query.bindValue(":privacyListName", privacyListName);
        query.exec();
    }
    return m_database.commit();
}

bool MySqlStorage::deletePrivacyList(QString jid, QString privacyListName)
{
    QSqlQuery query;
    query.prepare("DELETE FROM qjabberd_privacylist WHERE user_id = :user_id AND privacyListName = :privacyListName");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":privacyListName", privacyListName);
    return query.exec();
}

bool MySqlStorage::privacyListExist(QString jid, QString privacyListName)
{
    QSqlQuery query;
    query.prepare("SELECT privacyListName FROM qjabberd_privacylist WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    while (query.next())
    {
        if (query.value(0).toString() == privacyListName)
            return true;
    }
    return false;
}

QStringList MySqlStorage::getPrivacyListNames(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT privacyListName FROM qjabberd_privacylist WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    QStringList privacyListNames;
    while (query.next())
    {
        privacyListNames << query.value(0).toString();
    }
    return privacyListNames;
}

QString MySqlStorage::getDefaultPrivacyList(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT defaultPrivacyList FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    query.first();
    return query.value(0).toString();
}

QString MySqlStorage::getActivePrivacyList(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT activePrivacyList FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    query.first();
    return query.value(0).toString();
}

bool MySqlStorage::setDefaultPrivacyList(QString jid, QString defaultList)
{
    QSqlQuery query;
    query.prepare("UPDATE qjabberd_users SET defaultPrivacyList = :default WHERE jid = :jid");
    query.bindValue(":default", defaultList);
    query.bindValue(":jid", jid);
    return query.exec();
}

bool MySqlStorage::setActivePrivacyList(QString jid, QString activeList)
{
    QSqlQuery query;
    query.prepare("UPDATE qjabberd_users SET activePrivacyList = :active WHERE jid = :jid");
    query.bindValue(":active", activeList);
    query.bindValue(":jid", jid);
    return query.exec();
}

QList<PrivacyListItem> MySqlStorage::getPrivacyListItems(QString jid, QString privacyListName,
                                                         QString stanzaType, QString action)
{
    QSqlQuery query;
    query.prepare("SELECT type, value, action, iorder, child FROM qjabberd_privacylist WHERE user_id = :user_id"
                  " AND privacyListName = :privacyListName AND action = :action");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":privacyListName", privacyListName);
    query.bindValue(":action", action);
    query.exec();

    QList<PrivacyListItem> itemList;
    while (query.next())
    {
        QJsonDocument document = QJsonDocument::fromJson(query.value(4).toByteArray());
        QStringList childs = document.object().value("childs").toVariant().toStringList();
        if (childs.contains(stanzaType) || childs.isEmpty())
        {
            itemList << PrivacyListItem(query.value(0).toString(), query.value(1).toString(),
                                        query.value(2).toString(), query.value(3).toInt(),
                                        childs.toSet());
        }
    }
    return itemList;
}

QString MySqlStorage::getVCard(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT vcard FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    if (query.first())
    {
        return query.value(0).toString();
    }
    return QString();
}

bool MySqlStorage::updateVCard(QString jid, QString vCardInfos)
{
    QSqlQuery query;
    query.prepare("UPDATE qjabberd_users SET vcard = :vcard WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.bindValue(":vcard", vCardInfos);
    return query.exec();
}

bool MySqlStorage::vCardExist(QString jid)
{
    int user_id = getUserId(jid);
    if (user_id == -1)
        return false;

    QSqlQuery query;
    query.prepare("SELECT vcard FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    if (query.first())
        return !query.value(0).toString().isEmpty();

    return false;
}

QString MySqlStorage::getLastLogoutTime(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT lastLogoutTime FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    if (query.first())
    {
        query.value(0).toString();
    }
    return QString();
}

bool MySqlStorage::setLastLogoutTime(QString jid, QString lastLogoutTime)
{
    //qDebug() << "logout time : " << lastLogoutTime;
    QSqlQuery query;
    query.prepare("UPDATE qjabberd_users SET lastLogoutTime = :lastLogoutTime WHERE jid = :jid");
    query.bindValue(":lastLogoutTime", lastLogoutTime);
    query.bindValue(":jid", jid);
    return query.exec();
}

QString MySqlStorage::getLastStatus(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT lastStatus FROM qjabberd_users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    if (query.first())
    {
        query.value(0).toString();
    }
    return QString();
}

bool MySqlStorage::setLastStatus(QString jid, QString status)
{
    QSqlQuery query;
    query.prepare("UPDATE qjabberd_users SET lastStatus = :lastStatus WHERE jid = :jid");
    query.bindValue(":lastStatus", status);
    query.bindValue(":jid", jid);
    return query.exec();
}

bool MySqlStorage::storePrivateData(QString jid, QMultiHash<QString, QString> nodeMap)
{
    m_database.transaction();
    foreach (QString key, nodeMap.keys())
    {
        QSqlQuery query;
        query.prepare("INSERT INTO qjabberd_privatestorage(user_id, nodeName, nodeValue)"
                      " VALUES(:user_id, :nodeName, :nodeValue)");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":nodeName", key);
        query.bindValue(":nodeValue", nodeMap.value(key));
        query.exec();
    }
    return m_database.commit();
}

bool MySqlStorage::storePrivateData(QString jid, QList<MetaContact> metaContactList)
{
    m_database.transaction();
    foreach (MetaContact metacontact, metaContactList)
    {
        QSqlQuery query;
        query.prepare("INSERT INTO qjabberd_metacontact(user_id, jid, tag, order) VALUES(:user_id, :jid, :tag, :order)");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":jid", metacontact.getJid());
        query.bindValue(":tag", metacontact.getTag());
        query.bindValue(":order", metacontact.getOrder());
        query.exec();
    }
    return m_database.commit();
}

QByteArray MySqlStorage::getPrivateData(QString jid, QString node)
{
    QSqlQuery query;
    query.prepare("SELECT nodeValue FROM qjabberd_privatestorage WHERE user_id = :user_id AND nodeName = :nodeName");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":nodeName", node);
    query.exec();

    if (query.first())
    {
        return query.value(0).toByteArray();
    }
    return QByteArray();
}

QList<MetaContact> MySqlStorage::getPrivateData(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT jid, tag, order FROM qjabberd_metacontact WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    QList<MetaContact> metaContactList;
    while (query.next())
    {
        metaContactList << MetaContact(query.value(0).toString(),
                                       query.value(1).toString(),
                                       query.value(2).toInt());
    }
    return metaContactList;
}

bool MySqlStorage::saveOfflineMessage(QString from, QString to, QString type,
                                      QList<QPair<QString, QString> > bodyPairList, QString stamp)
{
    QJsonDocument document;
    QJsonObject object;
    QJsonArray bodyArray;
    for (int i = 0; i < bodyPairList.count(); ++i)
    {
        QJsonObject bodyObject;
        bodyObject.insert("lang", bodyPairList.value(i).first);
        bodyObject.insert("msg", bodyPairList.value(i).second);
        bodyArray.append(bodyObject);
    }
    object.insert("body", bodyArray);
    document.setObject(object);

    QSqlQuery query;
    query.prepare("INSERT INTO qjabberd_offlinemessage(user_id, ufrom, stamp, type, body)"
                  " VALUES(:user_id, :ufrom, :stamp, :type, :body)");
    query.bindValue(":user_id", getUserId(to));
    query.bindValue(":ufrom", from);
    query.bindValue(":stamp", stamp);
    query.bindValue(":type", type);
    query.bindValue(":body", document.toJson());
    return query.exec();
}

int MySqlStorage::getOfflineMessagesNumber(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM qjabberd_offlinemessage WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    if (query.first())
        return query.value(0).toInt();
    return -1;
}

QByteArray MySqlStorage::getOfflineMessage(QString jid, QString stamp)
{
    QSqlQuery query;
    query.prepare("SELECT ufrom, type, body FROM qjabberd_offlinemessage WHERE user_id = :user_id AND stamp = :stamp");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":stamp", stamp);
    query.exec();

    if (query.first())
    {
        QDomDocument document;
        QDomElement messageElement = document.createElement("message");
        messageElement.setAttribute("from", query.value(0).toString());
        messageElement.setAttribute("to", jid);
        messageElement.setAttribute("type", query.value(1).toString());

        QJsonArray bodyArray = QJsonDocument::fromJson(query.value(2).toByteArray()).object().value("body").toArray();
        for (int i = 0; i < bodyArray.count(); ++i)
        {
            QDomElement bodyElement = document.createElement("body");
            bodyElement.appendChild(document.createTextNode(bodyArray[i].toObject().value("msg").toString()));
            if (!bodyArray[i].toObject().value("lang").toString().isEmpty())
                bodyElement.setAttribute("xml:lang", bodyArray[i].toObject().value("lang").toString());

            messageElement.appendChild(bodyElement);
        }
        document.appendChild(messageElement);
        return document.toByteArray();
    }
    return QByteArray();
}

QMultiHash<QString, QByteArray> MySqlStorage::getOfflineMessageFrom(QString jid, QString from)
{
    QSqlQuery query;
    query.prepare("SELECT type, body, stamp FROM qjabberd_offlinemessage WHERE user_id = :user_id AND ufrom = :ufrom");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":ufrom", from);
    query.exec();

    QMultiHash<QString, QByteArray> offlineMessageList;
    while (query.next())
    {
        QDomDocument document;
        QDomElement messageElement = document.createElement("message");
        messageElement.setAttribute("from", from);
        messageElement.setAttribute("to", jid);
        messageElement.setAttribute("type", query.value(0).toString());

        QJsonArray bodyArray = QJsonDocument::fromJson(query.value(1).toByteArray()).object().value("body").toArray();
        for (int i = 0; i < bodyArray.count(); ++i)
        {
            QDomElement bodyElement = document.createElement("body");
            bodyElement.appendChild(document.createTextNode(bodyArray[i].toObject().value("msg").toString()));
            if (!bodyArray[i].toObject().value("lang").toString().isEmpty())
                bodyElement.setAttribute("xml:lang", bodyArray[i].toObject().value("lang").toString());

            messageElement.appendChild(bodyElement);
        }
        document.appendChild(messageElement);
        offlineMessageList.insert(query.value(2).toString(), document.toByteArray());
    }
    return offlineMessageList;
}

QMultiHash<QString, QByteArray> MySqlStorage::getAllOfflineMessage(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT ufrom, type, body, stamp FROM qjabberd_offlinemessage WHERE user_id = :user_id ORDER BY stamp ASC");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    QMultiHash<QString, QByteArray> offlineMessageList;
    while (query.next())
    {
        QDomDocument document;
        QDomElement messageElement = document.createElement("message");
        messageElement.setAttribute("from", query.value(0).toString());
        messageElement.setAttribute("to", jid);
        messageElement.setAttribute("type", query.value(1).toString());

        QJsonArray bodyArray = QJsonDocument::fromJson(query.value(2).toByteArray()).object().value("body").toArray();
        for (int i = 0; i < bodyArray.count(); ++i)
        {
            QDomElement bodyElement = document.createElement("body");
            bodyElement.appendChild(document.createTextNode(bodyArray[i].toObject().value("msg").toString()));
            if (!bodyArray[i].toObject().value("lang").toString().isEmpty())
                bodyElement.setAttribute("xml:lang", bodyArray[i].toObject().value("lang").toString());

            messageElement.appendChild(bodyElement);
        }
        document.appendChild(messageElement);
        offlineMessageList.insert(query.value(3).toString(), document.toByteArray());
    }
    return offlineMessageList;
}

bool MySqlStorage::deleteOfflineMessage(QString jid, QString key)
{
    if (key.contains("@"))
    {
        QSqlQuery query;
        query.prepare("DELETE FROM qjabberd_offlinemessage WHERE user_id = :user_id AND ufrom = :ufrom");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":ufrom", key);
        return query.exec();
    }
    else
    {
        QSqlQuery query;
        query.prepare("DELETE FROM qjabberd_offlinemessage WHERE user_id = :user_id AND stamp = :stamp");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":stamp", key);
        return query.exec();
    }
}

bool MySqlStorage::deleteAllOfflineMessage(QString jid)
{
    QSqlQuery query;
    query.prepare("DELETE FROM qjabberd_offlinemessage WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    return query.exec();
}

QMultiHash<QString, QString> MySqlStorage::getOfflineMessageHeaders(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT stamp, ufrom FROM qjabberd_offlinemessage WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    QMultiHash<QString, QString> offlineMessageHeaders;
    while (query.next())
    {
        offlineMessageHeaders.insert(query.value(0).toString(), query.value(1).toString());
    }
    return offlineMessageHeaders;
}

bool MySqlStorage::saveOfflinePresenceSubscription(QString from, QString to, QByteArray presence,
                                                   QString presenceType)
{
    QSqlQuery query;
    query.prepare("INSERT INTO qjabberd_offlinepresencesubscription(user_id, type, ufrom, uto, presenceStanza)"
                  " VALUES(:user_id, :type, :ufrom, :uto, :presenceStanza)");
    query.bindValue(":user_id", getUserId(to));
    query.bindValue(":ufrom", from);
    query.bindValue(":uto", to);
    query.bindValue(":type", presenceType);
    query.bindValue(":presenceStanza", presence);
    return query.exec();
}

QList<QVariant> MySqlStorage::getOfflinePresenceSubscription(QString jid)
{
    int user_id = getUserId(jid);
    QSqlQuery query;
    query.prepare("SELECT presenceStanza FROM qjabberd_offlinepresencesubscription WHERE user_id = :user_id");
    query.bindValue(":user_id", user_id);
    query.exec();

    QList<QVariant> subscriptionList;
    while (query.next())
    {
        subscriptionList << query.value(0);
    }

    query.prepare("DELETE FROM qjabberd_offlinepresencesubscription WHERE user_id = :user_id"
                  " AND (type = 'subscribed' OR type = 'unsubscribed' OR type = 'unsubscribe'");
    query.bindValue(":user_id", user_id);
    query.exec();

    return subscriptionList;
}

bool MySqlStorage::deleteOfflinePresenceSubscribe(QString from, QString to)
{
    QSqlQuery query;
    query.prepare("DELETE FROM qjabberd_offlinepresencesubscription WHERE ufrom = :ufrom AND uto = :uto");
    query.bindValue(":ufrom", from);
    query.bindValue(":uto", to);
    return query.exec();
}

QList<QString> MySqlStorage::getUserBlockList(QString jid)
{
    int user_id = getUserId(jid);

    QSqlQuery query;
    query.prepare("SELECT jid FROM qjabberd_blocklist WHERE user_id = :user_id");
    query.bindValue(":user_id", user_id);
    query.exec();

    QList<QString> items;
    while (query.next())
    {
        items << query.value(0).toString();
    }
    return items;
}

bool MySqlStorage::addUserBlockListItems(QString jid, QList<QString> items)
{
    /* Map the block list to the default privacy list */
//    QList<PrivacyListItem> privacyListItems;
//    foreach (QString item, items)
//    {
//        privacyListItems << PrivacyListItem("", item, "deny", 0,QSet<QString>());
//    }
//    addItemsToPrivacyList(jid, "default", privacyListItems);
    int user_id = getUserId(jid);

    m_database.transaction();
    foreach (QString item, items)
    {
        QSqlQuery query;
        query.prepare("INSERT INTO qjabberd_blocklist(user_id, jid) VALUES(:user_id, :jid)");
        query.bindValue(":user_id", user_id);
        query.bindValue(":jid", item);
        query.exec();
    }
    return m_database.commit();
}

bool MySqlStorage::deleteUserBlockListItems(QString jid, QList<QString> items)
{
    int user_id = getUserId(jid);

    m_database.transaction();
    foreach (QString item, items)
    {
        QSqlQuery query;
        query.prepare("DELETE FROM qjabberd_blocklist WHERE user_id = :user_id AND jid = :jid");
        query.bindValue(":user_id", user_id);
        query.bindValue(":jid", item);
        query.exec();
    }
    return m_database.commit();
}

bool MySqlStorage::emptyUserBlockList(QString jid)
{
    int user_id = getUserId(jid);

    QSqlQuery query;
    query.prepare("DELETE FROM qjabberd_blocklist WHERE user_id = :user_id");
    query.bindValue(":user_id", user_id);
    return query.exec();
}

//bool MySqlStorage::createRoom(QString roomName, QString ownerJid)
//{
//    return false;
//}

//QMultiHash<QString, QString> MySqlStorage::getChatRoomNameList(QString roomService)
//{
//    return QMultiHash<QString, QString>();
//}

//bool MySqlStorage::chatRoomExist(QString roomName)
//{
//    return false;
//}

//QStringList MySqlStorage::getOccupantsMucJid(QString roomName)
//{
//    return QStringList();
//}

//bool MySqlStorage::isPrivateOccupantsList(QString roomName)
//{
//    return false;
//}

//QList<Occupant> MySqlStorage::getOccupants(QString roomName)
//{
//    return QList<Occupant>();
//}

//QList<Occupant> MySqlStorage::getOccupants(QString roomName, QString bareJid)
//{

//}

//QString MySqlStorage::getOccupantMucJid(QString roomName, QString jid)
//{
//    return QString();
//}

//QString MySqlStorage::getOccupantJid(QString roomName, QString mucJid)
//{
//    return QString();
//}

//QString MySqlStorage::getOccupantRole(QString roomName, QString jid)
//{
//    return QString();
//}

//QString MySqlStorage::getOccupantRoleFromMucJid(QString roomName, QString mucJid)
//{
//    return QString();
//}

//QString MySqlStorage::getOccupantAffiliation(QString roomName, QString jid)
//{
//    return QString();
//}

//QString MySqlStorage::getOccupantAffiliationFromMucJid(QString roomName, QString mucJid)
//{
//    return QString();
//}

//Occupant MySqlStorage::getOccupant(QString roomName, QString jid)
//{
//    return Occupant();
//}

//bool MySqlStorage::addUserToRoom(QString roomName, Occupant occupant)
//{
//    return false;
//}

//QStringList MySqlStorage::getRoomTypes(QString roomName)
//{
//    return QStringList();
//}

//QString MySqlStorage::getRoomName(QString roomName)
//{
//    return QString();
//}

//bool MySqlStorage::isRegistered(QString roomName, QString jid)
//{
//    return false;
//}

//QStringList MySqlStorage::getRoomRegisteredMembersList(QString roomName)
//{
//    return QStringList();
//}

//bool MySqlStorage::isBannedUser(QString roomName, QString jid)
//{
//    return false;
//}

//bool MySqlStorage::nicknameOccuped(QString roomName, QString mucJid)
//{
//    return false;
//}

//bool MySqlStorage::maxOccupantsLimit(QString roomName)
//{
//    return false;
//}

//bool MySqlStorage::isLockedRoom(QString roomName)
//{
//    return false;
//}

//bool MySqlStorage::isPasswordProtectedRoom(QString roomName)
//{
//    return false;
//}

//QString MySqlStorage::getRoomPassword(QString roomName)
//{
//    return QString();
//}

//bool MySqlStorage::canBroadcastPresence(QString roomName, QString occupantRole)
//{
//    return false;
//}

//bool MySqlStorage::loggedDiscussion(QString roomName)
//{
//    return false;
//}

////QByteArray MySqlStorage::getMaxcharsHistory(QString roomName, int maxchar)
////{
////    return QByteArray();
////}

//QList<QDomDocument> MySqlStorage::getMaxstanzaHistory(QString roomName, int maxstanza)
//{
//    return QList<QDomDocument>();
//}

//QList<QDomDocument> MySqlStorage::getLastsecondsHistory(QString roomName, int seconds)
//{
//    return QList<QDomDocument>();
//}

//QList<QDomDocument> MySqlStorage::getHistorySince(QString roomName, QString since)
//{
//    return QList<QDomDocument>();
//}

//QList<QDomDocument> MySqlStorage::getHistorySinceMaxstanza(QString roomName, QString since, int maxstanza)
//{

//}

////QList<QDomDocument> MySqlStorage::getHistorySinceMaxchar(QString roomName, QString since, int maxchar)
////{

////}

////QList<QDomDocument> MySqlStorage::getHistorySinceSeconds(QString roomName, QString since, int seconds)
////{

////}

//QString MySqlStorage::getRoomSubject(QString roomName)
//{
//    return QString();
//}

//bool MySqlStorage::hasVoice(QString roomName, QString mucJid)
//{
//    return false;
//}

//bool MySqlStorage::changeRoomNickname(QString roomName, QString jid, QString nickname)
//{
//    return false;
//}

//bool MySqlStorage::changeRole(QString roomName, QString mucJid, QString newRole)
//{
//    return false;
//}

//bool MySqlStorage::registerUser(QString roomName, Occupant occupant)
//{
//    return false;
//}

//bool MySqlStorage::unlockRoom(QString roomName)
//{
//    return false;
//}

//bool MySqlStorage::submitConfigForm(QString roomName, QMap<QString, QVariant> dataFormValue)
//{
//    return false;
//}

//QStringList MySqlStorage::getRoomOwnersList(QString roomName)
//{
//    return QStringList();
//}

//QMap<QString, QVariant> MySqlStorage::getRoomConfig(QString roomName)
//{
//    return QMap<QString, QVariant>();
//}

//bool MySqlStorage::destroyRoom(QString roomName)
//{
//    return false;
//}

//QStringList MySqlStorage::getRoomModeratorsJid(QString roomName)
//{
//    return QStringList();
//}

//bool MySqlStorage::removeOccupantJid(QString roomName, QString jid)
//{
//    return false;
//}

//bool MySqlStorage::removeOccupant(QString roomName, QString mucJid)
//{
//    return false;
//}

//bool MySqlStorage::removeOccupants(QString roomName, QString bareJid)
//{

//}

//bool MySqlStorage::changeRoomSubject(QString roomName, QString subject)
//{
//    return false;
//}

//bool MySqlStorage::canChangeRoomSubject(QString roomName)
//{
//    return false;
//}

//QStringList MySqlStorage::getRoomAdminsList(QString roomName)
//{
//    return QStringList();
//}

//bool MySqlStorage::changeAffiliation(QString roomName, QString jid, QString newAffiliation)
//{
//    return false;
//}

//bool MySqlStorage::isPersistentRoom(QString roomName)
//{
//    return false;
//}

//bool MySqlStorage::changeOccupantStatus(QString roomName, QString mucJid, QString status)
//{

//}

//bool MySqlStorage::changeOccupantShow(QString roomName, QString mucJid, QString show)
//{

//}

//QString MySqlStorage::getOccupantStatusFromMucJid(QString roomName, QString mucJid)
//{

//}

//QString MySqlStorage::getOccupantShowFromMucJid(QString roomName, QString mucJid)
//{

//}

//Occupant MySqlStorage::getOccupantFromMucJid(QString roomName, QString mucJid)
//{

//}

//bool MySqlStorage::saveMucMessage(QString roomName, QByteArray message, QString stamp)
//{

//}

//int MySqlStorage::getRoomMaxhistoryFetch(QString roomName)
//{

//}

//QStringList MySqlStorage::getBannedList(QString roomName)
//{

//}
