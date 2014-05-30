#include "PgSqlStorage.h"

PgSqlStorage::PgSqlStorage(QString host, int port, QString username, QString password, QString database)
{
    m_database = QSqlDatabase::addDatabase("QPSQL");
    m_database.setHostName(host);
    m_database.setPort(port);
    m_database.setUserName(username);
    m_database.setPassword(password);
    m_database.setDatabaseName(database);
    if (m_database.open())
    {
        qDebug() << "Database open";
    }
    else{
        qDebug() << "Database not open";
    }
}

QString PgSqlStorage::getStorageType()
{
    return "PgSql";
}

int PgSqlStorage::getUserId(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE jid = :jid");
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

QString PgSqlStorage::getPassword(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    query.first();
    return query.value(0).toString();
}

bool PgSqlStorage::changePassword(QString jid, QString newPassword)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET password = :password WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.bindValue(":password", newPassword);
    return query.exec();
}

bool PgSqlStorage::createUser(QString jid, QString password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users(jid, password) VALUES(:jid, :password)");
    query.bindValue(":jid", jid);
    query.bindValue(":password", password);
    return query.exec();
}

bool PgSqlStorage::deleteUser(QString jid)
{
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    return query.exec();
}

QList<Contact> PgSqlStorage::getContactsList(QString jid)
{
    QList<Contact> contactList;
    QSqlQuery query;
    query.prepare("SELECT version, approved, ask, jid, name, subscription, groups"
                  " FROM contact WHERE user_id = :user_id");
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

bool PgSqlStorage::addContactToRoster(QString jid, Contact contact)
{
    if (contactExists(jid, contact.getJid()))
    {
        updateNameToContact(jid, contact.getJid(), contact.getName());
        updateGroupToContact(jid, contact.getJid(), contact.getGroups());
        return true;
    }
    else
    {
        qDebug() << "Je suis juste ici : " << getUserId(jid);
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

bool PgSqlStorage::deleteContactToRoster(QString jid, QString contactJid)
{
    QSqlQuery query;
    query.prepare("DELETE FROM contact WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    return query.exec();
}

bool PgSqlStorage::updateGroupToContact(QString jid, QString contactJid,
                                         QSet<QString> groups)
{
    QJsonDocument document;
    QJsonObject object;
    object.insert("groups", QJsonArray::fromStringList(QStringList::fromSet(groups)));
    document.setObject(object);

    QSqlQuery query;
    query.prepare("UPDATE contact SET groups = :groups WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.bindValue(":groups", document.toJson());
    return query.exec();
}

bool PgSqlStorage::updateSubscriptionToContact(QString jid, QString contactJid,
                                         QString subscription)
{
    if (!contactExists(jid, contactJid))
    {
        Contact contact("", false, "", contactJid, "", subscription, QSet<QString>());
        return addContactToRoster(jid, contact);
    }

    QString contactSubscription = getContact(jid, contactJid).getSubscription();

    QSqlQuery query;
    query.prepare("UPDATE contact SET subscription = :subscription WHERE user_id = :user_id AND jid = :jid");
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

bool PgSqlStorage::updateAskAttributeToContact(QString jid, QString contactJid,
                                         QString ask)
{
    Contact contact = getContact(jid, contactJid);

    QSqlQuery query;
    query.prepare("UPDATE contact SET ask = :ask WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);

    if ((contact.getSubscription() != "to") && (contact.getSubscription() != "both"))
    {
        query.bindValue(":ask", ask);
    }
    return query.exec();
}

bool PgSqlStorage::updateNameToContact(QString jid, QString contactJid,
                                         QString name)
{
    QSqlQuery query;
    query.prepare("UPDATE contact SET name = :name WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.bindValue(":name", name);
    return query.exec();
}

bool PgSqlStorage::updateApprovedToContact(QString jid, QString contactJid, bool approved)
{
    QSqlQuery query;
    query.prepare("UPDATE contact SET approved = :approved WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.bindValue(":approved", approved);
    return query.exec();
}

bool PgSqlStorage::userExists(QString jid)
{
    return (getUserId(jid) >= 0);
}

bool PgSqlStorage::contactExists(QString jid, QString contactJid)
{
    int user_id = getUserId(jid);

    if (user_id == -1)
        return false;

    QSqlQuery query;
    query.prepare("SELECT id FROM contact WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.exec();
    return query.first();
}

Contact PgSqlStorage::getContact(QString jid, QString contactJid)
{
    QSqlQuery query;
    query.prepare("SELECT version, approved, ask, jid, name, subscription, groups FROM contact WHERE user_id = :user_id AND jid = :jid");
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

QSet<QString> PgSqlStorage::getContactGroups(QString jid, QString contactJid)
{
    QSqlQuery query;
    query.prepare("SELECT group FROM contact WHERE user_id = :user_id AND jid = :jid");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":jid", contactJid);
    query.exec();
}

QList<PrivacyListItem> PgSqlStorage::getPrivacyList(QString jid, QString privacyListName)
{
    QSqlQuery query;
    query.prepare("SELECT type, value, action, order, child FROM privacylist WHERE user_id = :user_id AND privacyListName = :privacyListName");
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
    return itemList;
}

bool PgSqlStorage::addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items)
{
    m_database.transaction();

    QJsonDocument document;
    QJsonObject object;
    foreach (PrivacyListItem item, items)
    {
        object.insert("childs", QJsonArray::fromStringList(QStringList::fromSet(item.getChildsElement())));
        document.setObject(object);

        QSqlQuery query;
        query.prepare("INSERT INTO privacylist(user_id, type, action, order, child, privacyListName)"
                      "VALUES(:user_id, :type, :action, :order, :child, :privacyListName)");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":type", item.getType());
        query.bindValue(":action", item.getAction());
        query.bindValue(":order", item.getOrder());
        query.bindValue(":child", document.toJson());
        query.bindValue(":privacyListName", privacyListName);
        query.exec();
    }
    return m_database.commit();
}

bool PgSqlStorage::deletePrivacyList(QString jid, QString privacyListName)
{
    QSqlQuery query;
    query.prepare("DELETE FROM privacylist WHERE user_id = :user_id AND privacyListName = :privacyListName");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":privacyListName", privacyListName);
    return query.exec();
}

QString PgSqlStorage::getVCard(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT vcard FROM users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    if (query.first())
    {
        return query.value(0).toString();
    }
    return QString();
}

bool PgSqlStorage::updateVCard(QString jid, QString vCardInfos)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET vcard = :vcard WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.bindValue(":vcard", vCardInfos);
    return query.exec();
}

bool PgSqlStorage::vCardExist(QString jid)
{
    int user_id = getUserId(jid);
    if (user_id == -1)
        return false;

    QSqlQuery query;
    query.prepare("SELECT vcard FROM users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    if (query.first())
        return !query.value(0).toString().isEmpty();

    return false;
}

QString PgSqlStorage::getLastLogoutTime(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT lastLogoutTime FROM users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    if (query.first())
    {
        query.value(0).toString();
    }
    return QString();
}

bool PgSqlStorage::setLastLogoutTime(QString jid, QString lastLogoutTime)
{
    qDebug() << "logout time : " << lastLogoutTime;
    QSqlQuery query;
    query.prepare("UPDATE users SET lastLogoutTime = :lastLogoutTime WHERE jid = :jid");
    query.bindValue(":lastLogoutTime", lastLogoutTime);
    query.bindValue(":jid", jid);
    return query.exec();
}

QString PgSqlStorage::getLastStatus(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT lastStatus FROM users WHERE jid = :jid");
    query.bindValue(":jid", jid);
    query.exec();

    if (query.first())
    {
        query.value(0).toString();
    }
    return QString();
}

bool PgSqlStorage::setLastStatus(QString jid, QString status)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET lastStatus = :lastStatus WHERE jid = :jid");
    query.bindValue(":lastStatus", status);
    query.bindValue(":jid", jid);
    return query.exec();
}

bool PgSqlStorage::storePrivateData(QString jid, QMultiHash<QString, QString> nodeMap)
{
    m_database.transaction();
    foreach (QString key, nodeMap.keys())
    {
        QSqlQuery query;
        query.prepare("INSERT INTO privatedata(user_id, nodeName, nodeValue)"
                      " VALUES(:user_id, :nodeName, :nodeValue)");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":nodeName", key);
        query.bindValue(":nodeValue", nodeMap.value(key));
        query.exec();
    }
    return m_database.commit();
}

bool PgSqlStorage::storePrivateData(QString jid, QList<MetaContact> metaContactList)
{
    m_database.transaction();
    foreach (MetaContact metacontact, metaContactList)
    {
        QSqlQuery query;
        query.prepare("INSERT INTO metacontact(user_id, jid, tag, order) VALUES(:user_id, :jid, :tag, :order)");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":jid", metacontact.getJid());
        query.bindValue(":tag", metacontact.getTag());
        query.bindValue(":order", metacontact.getOrder());
        query.exec();
    }
    return m_database.commit();
}

QByteArray PgSqlStorage::getPrivateData(QString jid, QString node)
{
    QSqlQuery query;
    query.prepare("SELECT nodeValue FROM privatedata WHERE user_id = :user_id AND nodeName = :nodeName");
    query.bindValue(":user_id", getUserId(jid));
    query.bindValue(":nodeName", node);
    query.exec();

    if (query.first())
    {
        return query.value(0).toByteArray();
    }
    return QByteArray();
}

QList<MetaContact> PgSqlStorage::getPrivateData(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT jid, tag, order FROM metacontact WHERE user_id = :user_id");
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

bool PgSqlStorage::saveOfflineMessage(QString from, QString to, QString type,
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
    query.prepare("INSERT INTO offlinemessage(user_id, ufrom, stamp, type, body)"
                  " VALUES(:user_id, :ufrom, :stamp, :type, :body)");
    query.bindValue(":user_id", getUserId(to));
    query.bindValue(":ufrom", from);
    query.bindValue(":stamp", stamp);
    query.bindValue(":type", type);
    query.bindValue(":body", document.toJson());
    return query.exec();
}

int PgSqlStorage::getOfflineMessagesNumber(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM offlineMessage WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    if (query.first())
        return query.value(0).toInt();
    return -1;
}

QByteArray PgSqlStorage::getOfflineMessage(QString jid, QString stamp)
{
    QSqlQuery query;
    query.prepare("SELECT ufrom, type, body FROM offlineMessage WHERE user_id = :user_id AND stamp = :stamp");
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

QMultiHash<QString, QByteArray> PgSqlStorage::getOfflineMessageFrom(QString jid, QString from)
{
    QSqlQuery query;
    query.prepare("SELECT type, body, stamp FROM offlineMessage WHERE user_id = :user_id AND ufrom = :ufrom");
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

QMultiHash<QString, QByteArray> PgSqlStorage::getAllOfflineMessage(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT ufrom, type, body, stamp FROM offlineMessage WHERE user_id = :user_id ORDER BY stamp ASC");
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

bool PgSqlStorage::deleteOfflineMessage(QString jid, QString key)
{
    if (key.contains("@"))
    {
        QSqlQuery query;
        query.prepare("DELETE FROM offlinemessage WHERE user_id = :user_id AND ufrom = :ufrom");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":ufrom", key);
        return query.exec();
    }
    else
    {
        QSqlQuery query;
        query.prepare("DELETE FROM offlinemessage WHERE user_id = :user_id AND stamp = :stamp");
        query.bindValue(":user_id", getUserId(jid));
        query.bindValue(":stamp", key);
        return query.exec();
    }
}

bool PgSqlStorage::deleteAllOfflineMessage(QString jid)
{
    QSqlQuery query;
    query.prepare("DELETE FROM offlinemessage WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    return query.exec();
}

QMultiHash<QString, QString> PgSqlStorage::getOfflineMessageHeaders(QString jid)
{
    QSqlQuery query;
    query.prepare("SELECT stamp, ufrom FROM offlinemessage WHERE user_id = :user_id");
    query.bindValue(":user_id", getUserId(jid));
    query.exec();

    QMultiHash<QString, QString> offlineMessageHeaders;
    while (query.next())
    {
        offlineMessageHeaders.insert(query.value(0).toString(), query.value(1).toString());
    }
    return offlineMessageHeaders;
}

bool PgSqlStorage::saveOfflinePresenceSubscription(QString from, QString to, QByteArray presence,
                                                   QString presenceType)
{
    QSqlQuery query;
    query.prepare("INSERT INTO offlinepresencesubscription(user_id, type, ufrom, uto, presenceStanza)"
                  " VALUES(:user_id, :type, :ufrom, :uto, :presenceStanza)");
    query.bindValue(":user_id", getUserId(to));
    query.bindValue(":ufrom", from);
    query.bindValue(":uto", to);
    query.bindValue(":type", presenceType);
    query.bindValue(":presenceStanza", presence);
    return query.exec();
}

QList<QVariant> PgSqlStorage::getOfflinePresenceSubscription(QString jid)
{
    int user_id = getUserId(jid);
    QSqlQuery query;
    query.prepare("SELECT presenceStanza FROM offlinepresencesubscription WHERE user_id = :user_id");
    query.bindValue(":user_id", user_id);
    query.exec();

    QList<QVariant> subscriptionList;
    while (query.next())
    {
        subscriptionList << query.value(0);
    }

    query.prepare("DELETE FROM offlinepresencesubscription WHERE user_id = :user_id"
                  " AND (type = 'subscribed' OR type = 'unsubscribed' OR type = 'unsubscribe'");
    query.bindValue(":user_id", user_id);
    query.exec();

    return subscriptionList;
}

bool PgSqlStorage::deleteOfflinePresenceSubscribe(QString from, QString to)
{
    QSqlQuery query;
    query.prepare("DELETE FROM offlinepresencesubscription WHERE ufrom = :ufrom AND uto = :uto");
    query.bindValue(":ufrom", from);
    query.bindValue(":uto", to);
    return query.exec();
}

QList<QString> PgSqlStorage::getUserBlockList(QString jid)
{
    return QList<QString>();
}

bool PgSqlStorage::addUserBlockListItems(QString jid, QList<QString> items)
{
    /* Map the block list to the default privacy list */
    QList<PrivacyListItem> privacyListItems;
    foreach (QString item, items)
    {
        privacyListItems << PrivacyListItem("", item, "deny", 0,QSet<QString>());
    }
    addItemsToPrivacyList(jid, "default", privacyListItems);

    return true;
}

bool PgSqlStorage::deleteUserBlockListItems(QString jid, QList<QString> items)
{
    return true;
}

bool PgSqlStorage::emptyUserBlockList(QString jid)
{
    return true;
}

QMultiHash<QString, QString> PgSqlStorage::getChatRoomNameList(QString roomService)
{
    return QMultiHash<QString, QString>();
}

bool PgSqlStorage::chatRoomExist(QString roomName)
{
    return false;
}


QList<QString> PgSqlStorage::getChatRoomOccupants(QString roomName)
{
    return QList<QString>();
}

bool PgSqlStorage::isPrivateOccupantsList(QString roomName)
{
    return false;
}

