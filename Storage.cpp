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

    QJsonDocument document = QJsonDocument::fromJson(userStreamDataFile.readAll());
    QJsonObject userStreamDataObject = document.object();

    QJsonArray userStreamDataArray = userStreamDataObject.value("data").toArray();
    userStreamDataArray.append(QJsonValue(QString(data)));

    userStreamDataObject.insert("stanza", userStreamDataArray);
    document.setObject(userStreamDataObject);

    userStreamDataFile.resize(0);
    bool ok = userStreamDataFile.write(document.toJson());
    userStreamDataFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::saveStreamPresencePriority(QString smId, int presencePriority)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userStreamDataFile.readAll());
    QJsonObject userStreamDataObject = document.object();

    userStreamDataObject.insert("presencePriority", presencePriority);
    document.setObject(userStreamDataObject);

    userStreamDataFile.resize(0);
    bool ok = userStreamDataFile.write(document.toJson());
    userStreamDataFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::saveStreamPresenceStanza(QString smId, QByteArray presenceData)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userStreamDataFile.readAll());
    QJsonObject userStreamDataObject = document.object();

    userStreamDataObject.insert("presenceStanza", QJsonValue(QString(presenceData)));
    document.setObject(userStreamDataObject);

    userStreamDataFile.resize(0);
    bool ok = userStreamDataFile.write(document.toJson());
    userStreamDataFile.close();
    return (true ? (ok >= 0) : false);
}

bool Storage::deleteStreamData(QString smId, int h)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return false;

    QJsonDocument document = QJsonDocument::fromJson(userStreamDataFile.readAll());
    QJsonObject userStreamDataObject = document.object();

    QJsonArray userStreamDataArray = userStreamDataObject.value("data").toArray();
    userStreamDataArray.removeAt(h - 1);

    userStreamDataObject.insert("stanza", userStreamDataArray);
    document.setObject(userStreamDataObject);

    userStreamDataFile.resize(0);
    bool ok = userStreamDataFile.write(document.toJson());
    userStreamDataFile.close();
    return (true ? (ok >= 0) : false);
}

QList<QByteArray> Storage::getClientUnhandleStanza(QString smId)
{
    QString filename = "temp/" + smId + ".qjt";
    QFile userStreamDataFile(filename);

    if (!userStreamDataFile.open(QIODevice::ReadWrite))
        return QList<QByteArray>();

    QJsonArray stanzaArray =  QJsonDocument::fromJson(userStreamDataFile.readAll()).object().value("stanza").toArray();
    QList<QByteArray> stanzaList;

    for (int i = 0; i < stanzaArray.count(); ++i)
    {
        stanzaList << stanzaArray[i].toString().toUtf8();
    }
    return stanzaList;
}
