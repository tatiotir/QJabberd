#include "Storage.h"

Storage::Storage()
{

}

bool Storage::saveStreamData(QString smId, QByteArray data)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(userStreamDataFile.readAll());
    QJsonObject userStreamDataObject = document.object();

    QJsonArray userStreamDataArray = userStreamDataObject.value("data").toArray();
    userStreamDataArray.append(QJsonValue(QString(data)));

    userStreamDataObject.insert("stanza", userStreamDataArray);
    document.setObject(userStreamDataObject);

    userStreamDataFile.resize(0);
    bool ok = userStreamDataFile.write(document.toBinaryData());
    userStreamDataFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::saveStreamPresencePriority(QString smId, int presencePriority)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(userStreamDataFile.readAll());
    QJsonObject userStreamDataObject = document.object();

    userStreamDataObject.insert("presencePriority", presencePriority);
    document.setObject(userStreamDataObject);

    userStreamDataFile.resize(0);
    bool ok = userStreamDataFile.write(document.toBinaryData());
    userStreamDataFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::saveStreamPresenceStanza(QString smId, QByteArray presenceData)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(userStreamDataFile.readAll());
    QJsonObject userStreamDataObject = document.object();

    userStreamDataObject.insert("presenceStanza", QJsonValue(QString(presenceData)));
    document.setObject(userStreamDataObject);

    userStreamDataFile.resize(0);
    bool ok = userStreamDataFile.write(document.toBinaryData());
    userStreamDataFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::deleteStreamData(QString smId, int h)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(userStreamDataFile.readAll());
    QJsonObject userStreamDataObject = document.object();

    QJsonArray userStreamDataArray = userStreamDataObject.value("data").toArray();
    userStreamDataArray.removeAt(h - 1);

    userStreamDataObject.insert("stanza", userStreamDataArray);
    document.setObject(userStreamDataObject);

    userStreamDataFile.resize(0);
    bool ok = userStreamDataFile.write(document.toBinaryData());
    userStreamDataFile.close();
    return (true ? (ok >= 0) : false);
}

QList<QByteArray> Storage::getClientUnhandleStanza(QString smId)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return QList<QByteArray>();

    QJsonArray stanzaArray =  QJsonDocument::fromBinaryData(userStreamDataFile.readAll()).object().value("stanza").toArray();
    QList<QByteArray> stanzaList;

    for (int i = 0; i < stanzaArray.count(); ++i)
    {
        stanzaList << stanzaArray[i].toString().toUtf8();
    }
    return stanzaList;
}

bool Storage::createRoom(QString roomName, QString ownerJid)
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

    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

QMultiHash<QString, QString> Storage::getChatRoomNameList(QString roomService)
{
    QDir dir(roomService);
    QStringList roomFilenameList = dir.entryList(QDir::Files | QDir::NoDot | QDir::NoDotAndDotDot | QDir::NoDotDot);

    QMultiHash<QString, QString> map;
    foreach (QString roomFilename, roomFilenameList)
    {
        QFile roomFile(roomService + "/" + roomFilename);
        roomFile.open(QIODevice::ReadOnly);

        QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());

        if (document.object().value("roomConfig").toObject().value("muc#roomconfig_publicroom").toVariant().toBool())
        {
            QString roomName = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomname").toString();
            if (roomName.isEmpty())
                roomName = Utils::getUsername(document.object().value("roomName").toString());

            map.insert(QFileInfo(roomFilename).completeBaseName().remove("_" + roomService) + "@" + roomService, roomName);
        }
        roomFile.close();
    }
    return map;
}

bool Storage::chatRoomExist(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    return QFile::exists(filename);
}

QStringList Storage::getOccupantsMucJid(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QList<QString>();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QStringList occupantMucJid;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        occupantMucJid << occupantArray[i].toObject().value("mucJid").toString();
    }
    roomFile.close();
    return occupantMucJid;
}

bool Storage::isPrivateOccupantsList(QString roomName)
{
    return false;
}

QList<Occupant> Storage::getOccupants(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QList<Occupant>();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QJsonArray occupantArray = document.object().value("occupants").toArray();

    QList<Occupant> occupantsList;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        occupantsList << Occupant::fromJsonObject(occupantArray[i].toObject());
    }
    roomFile.close();
    return occupantsList;
}

QList<Occupant> Storage::getOccupants(QString roomName, QString bareJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QList<Occupant>();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

QString Storage::getOccupantMucJid(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

QString Storage::getOccupantJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

QString Storage::getOccupantRole(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

QString Storage::getOccupantRoleFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

QString Storage::getOccupantAffiliation(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

QString Storage::getOccupantAffiliationFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

Occupant Storage::getOccupant(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return Occupant();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

bool Storage::addUserToRoom(QString roomName, Occupant occupant)
{
//    if (getOccupantMucJid(roomName, occupant.jid()) == occupant.mucJid())
//        return true;

    // Remove this occupant if it exist in the room
    removeOccupantJid(roomName, occupant.jid());

    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QJsonObject roomObject = document.object();

    QJsonArray occupantsArray = roomObject.value("occupants").toArray();
    occupantsArray.append(occupant.toJsonObject());

    roomObject.insert("occupants", occupantsArray);
    document.setObject(roomObject);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

QStringList Storage::getRoomTypes(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QStringList roomType = document.object().value("roomTypes").toVariant().toStringList();
    roomFile.close();
    return roomType;
}

QString Storage::getRoomName(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QString naturalRoomName = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomName").toString();
    roomFile.close();
    return naturalRoomName;
}

bool Storage::isRegistered(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    bool registered = document.object().value("memberList").toVariant().toStringList().contains(jid);
    roomFile.close();
    return registered;
}

QStringList Storage::getRoomRegisteredMembersList(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QStringList registerdMemberMist = document.object().value("memberList").toVariant().toStringList();
    roomFile.close();
    return registerdMemberMist;
}

bool Storage::isBannedUser(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    bool banned = document.object().value("bannedList").toVariant().toStringList().contains(jid);
    roomFile.close();
    return banned;
}

bool Storage::nicknameOccuped(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

bool Storage::maxOccupantsLimit(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    int maxUsersConfig = document.object().value("roomConfig").toObject().value("muc#roomconfig_maxusers").toString().toInt();
    int numberOccupants = document.object().value("occupants").toArray().count();

    roomFile.close();
    return (numberOccupants == maxUsersConfig);
}

bool Storage::isLockedRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    bool locked = document.object().value("locked").toBool();
    roomFile.close();
    return locked;
}

bool Storage::isPasswordProtectedRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    bool passwordprotected = document.object().value("roomConfig").toObject().value("muc#roomconfig_passwordprotectedroom").toVariant().toBool();
    roomFile.close();
    return passwordprotected;
}

QString Storage::getRoomPassword(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QString password = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomsecret").toString();
    roomFile.close();
    return password;
}

bool Storage::canBroadcastPresence(QString roomName, QString occupantRole)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    bool canbroadcast = document.object().value("roomConfig").toObject()
            .value("muc#roomconfig_presencebroadcast").toVariant().toStringList().contains(occupantRole);
    roomFile.close();
    return canbroadcast;
}

bool Storage::loggedDiscussion(QString roomName)
{
    return false;
}

//QByteArray Storage::getMaxcharsHistory(QString roomName, int maxchar)
//{
//    QString roomService = Utils::getHost(roomName);
//    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";

//    QFile historyFile(historyFilename);
//    if (!historyFile.open(QIODevice::ReadOnly))
//        return false;

//    QJsonDocument document = QJsonDocument::fromBinaryData(historyFile.readAll());

//    QByteArray data;
//    QList<QVariant> historyList = document.object().toVariantMap().values();
//    for (int i = 0; i < historyList.count(); ++i)
//    {
//        data.append(historyList.value(i).toByteArray());
//    }
//    return data.left(maxchar);
//}

QList<QDomDocument> Storage::getMaxstanzaHistory(QString roomName, int maxstanza)
{
    QString roomService = Utils::getHost(roomName);
    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";

    QFile historyFile(historyFilename);
    if (!historyFile.open(QIODevice::ReadOnly))
        return QList<QDomDocument>();

    QJsonDocument document = QJsonDocument::fromBinaryData(historyFile.readAll());

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

QList<QDomDocument> Storage::getLastsecondsHistory(QString roomName, int seconds)
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

    QJsonDocument historyDocument = QJsonDocument::fromBinaryData(historyFile.readAll());
    QJsonDocument historyIndexDocument = QJsonDocument::fromBinaryData(historyIndexFile.readAll());

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

QList<QDomDocument> Storage::getHistorySince(QString roomName, QString since)
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

    QJsonDocument historyDocument = QJsonDocument::fromBinaryData(historyFile.readAll());
    QJsonDocument historyIndexDocument = QJsonDocument::fromBinaryData(historyIndexFile.readAll());

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

QList<QDomDocument> Storage::getHistorySinceMaxstanza(QString roomName, QString since, int maxstanza)
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

    QJsonDocument historyDocument = QJsonDocument::fromBinaryData(historyFile.readAll());
    QJsonDocument historyIndexDocument = QJsonDocument::fromBinaryData(historyIndexFile.readAll());

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

//QList<QDomDocument> Storage::getHistorySinceMaxchar(QString roomName, QString since, int maxchar)
//{
//    QList<QDomDocument>();
//}

//QList<QDomDocument> Storage::getHistorySinceSeconds(QString roomName, QString since, int seconds)
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

//    QJsonDocument historyDocument = QJsonDocument::fromBinaryData(historyFile.readAll());
//    QJsonDocument historyIndexDocument = QJsonDocument::fromBinaryData(historyIndexFile.readAll());

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

QString Storage::getRoomSubject(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QString subject = document.object().value("roomSubject").toString();
    roomFile.close();
    return subject;
}

bool Storage::hasVoice(QString roomName, QString mucJid)
{
    QString role = getOccupantRoleFromMucJid(roomName, mucJid);
    if ((role == "none") || (role == "visitor"))
        return false;
    return true;
}

bool Storage::changeRoomNickname(QString roomName, QString jid, QString nickname)
{
    QString rName = roomName;
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + rName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

    occupantObject.insert("mucJid", roomName + "/" + nickname);
    occupantArray.append(occupantObject);
    object.insert("occupants", occupantArray);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toBinaryData());
    return (true ? (ok >= 0) : false);
}

bool Storage::changeRole(QString roomName, QString mucJid, QString newRole)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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
    quint64 ok = roomFile.write(document.toBinaryData());
    return (true ? (ok >= 0) : false);
}

bool Storage::registerUser(QString roomName, Occupant occupant)
{
    addUserToRoom(roomName, occupant);

    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QJsonObject object = document.object();

    QStringList memberList = object.value("memberList").toVariant().toStringList();
    memberList << Utils::getBareJid(occupant.jid());

    object.insert("memberList", QJsonArray::fromStringList(memberList));
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toBinaryData());
    return (true ? (ok >= 0) : false);
}

bool Storage::unlockRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QJsonObject object = document.object();
    object.insert("locked", false);

    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toBinaryData());
    return (true ? (ok >= 0) : false);
}

bool Storage::submitConfigForm(QString roomName, QMap<QString, QVariant> dataFormValue)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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
    quint64 ok = roomFile.write(document.toBinaryData());
    return (true ? (ok >= 0) : false);
}

QStringList Storage::getRoomOwnersList(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QList<QString>();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QStringList ownersList = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomowners").toVariant().toStringList();
    roomFile.close();
    return ownersList;
}

QMap<QString, QVariant> Storage::getRoomConfig(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QMap<QString, QVariant>();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QMap<QString, QVariant> roomConfig = document.object().value("roomConfig").toVariant().toMap();
    roomFile.close();
    return roomConfig;
}

bool Storage::destroyRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile;
    return roomFile.remove(filename);
}

QStringList Storage::getRoomModeratorsJid(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

bool Storage::removeOccupantJid(QString roomName, QString jid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QJsonObject object = document.object();

    QJsonArray occupantArray = object.value("occupants").toArray();
    QList<int> index;
    for (int i = 0; i < occupantArray.count(); ++i)
    {
        if (occupantArray[i].toObject().value("jid").toString() == jid)
            index << i;
    }

    for (int i = 0; i < index.count(); ++i)
        occupantArray.removeAt(index[i]);

    object.insert("occupants", occupantArray);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::removeOccupant(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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
    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::removeOccupants(QString roomName, QString bareJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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
    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::changeRoomSubject(QString roomName, QString subject)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QJsonObject object = document.object();
    object.insert("roomSubject", subject);
    document.setObject(object);

    roomFile.resize(0);
    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::canChangeRoomSubject(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    bool canChangeRoomSubject = document.object().value("roomConfig").toObject().value("muc#roomconfig_changesubject").toVariant().toBool();
    roomFile.close();
    return canChangeRoomSubject;
}

QStringList Storage::getRoomAdminsList(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QStringList adminList = document.object().value("roomConfig").toObject().value("muc#roomconfig_roomadmins").toVariant().toStringList();
    roomFile.close();
    return adminList;
}

bool Storage::changeAffiliation(QString roomName, QString jid, QString newAffiliation)
{
    QStringList roomTypes = getRoomTypes(roomName);
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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
    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::isPersistentRoom(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    bool isPersistent = document.object().value("roomConfig").toObject().value("muc#roomconfig_persistentroom").toVariant().toBool();
    roomFile.close();
    return isPersistent;
}

bool Storage::changeOccupantStatus(QString roomName, QString mucJid, QString status)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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
    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::changeOccupantShow(QString roomName, QString mucJid, QString show)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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
    quint64 ok = roomFile.write(document.toBinaryData());
    roomFile.close();
    return (true ? (ok >= 0) : false);
}

QString Storage::getOccupantStatusFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

QString Storage::getOccupantShowFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

Occupant Storage::getOccupantFromMucJid(QString roomName, QString mucJid)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return Occupant();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
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

bool Storage::saveMucMessage(QString roomName, QByteArray message, QString stamp)
{
    QString roomService = Utils::getHost(roomName);
    QDir dir(roomService);
    dir.mkdir("loggedMessage");

    QString historyFilename = roomService + "/loggedMessage/" + roomName.replace("@", "_") + ".qjh";
    QString historyIndexFilename = roomService + "/loggedMessage/" + roomName + ".qji";

    QFile historyFile(historyFilename);
    if (!historyFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument historyDocument = QJsonDocument::fromBinaryData(historyFile.readAll());
    QJsonObject object = historyDocument.object();

    QList<QString> keys = object.toVariantMap().keys();
    QString key = QString::number(keys.value(keys.count() - 1).toInt() + 1);

    object.insert(key, QString(message));
    historyDocument.setObject(object);

    historyFile.resize(0);
    bool ok = historyFile.write(historyDocument.toBinaryData());
    historyFile.close();

    // Build index
    QFile historyIndexFile(historyIndexFilename);
    if (!historyIndexFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument historyIndexDocument = QJsonDocument::fromBinaryData(historyIndexFile.readAll());
    QJsonObject indexObject = historyIndexDocument.object();

    indexObject.insert(stamp, key);
    historyIndexDocument.setObject(indexObject);

    historyIndexFile.resize(0);
    bool ok1 = historyIndexFile.write(historyIndexDocument.toBinaryData());
    historyIndexFile.close();

    return (ok && ok1);
}

int Storage::getRoomMaxhistoryFetch(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return 0;

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    roomFile.close();


    return document.object().value("roomConfig").toObject().value("muc#maxhistoryfetch").toVariant().toInt();
}

QStringList Storage::getBannedList(QString roomName)
{
    QString roomService = Utils::getHost(roomName);
    QString filename = roomService + "/" + roomName.replace("@", "_") + ".qjr";
    QFile roomFile(filename);

    if (!roomFile.open(QIODevice::ReadOnly))
        return QStringList();

    QJsonDocument document = QJsonDocument::fromBinaryData(roomFile.readAll());
    QStringList bannedList = document.object().value("bannedList").toVariant().toStringList();
    roomFile.close();

    return bannedList;
}
