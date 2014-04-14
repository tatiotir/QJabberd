#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QStringList>
#include <QTime>
#include <QSet>
#include <QCryptographicHash>
#include <QDomElement>
#include <QDomDocument>
#include <QDebug>

class Utils
{
public:
    Utils();

    static QByteArray digestCalculator(QString id, QString password);
    static QString generateResource();
    static QString generateId();
    static QString getBareJid(QString fullJid);
    static QString getResource(QString fullJid);
    static QString getUsername(QString fullJid);
    static QString getHost(QString fullJid);
    static QByteArray generateRosterPush(QString to, QString id, QString jid, QString name,
                                            QString subscription, QString ask, bool approved, QSet<QString> groupList);
    static QByteArray generatePresence(QString type, QString from, QString to, QString id, QString show,
                                       QString priority, QMultiHash<QString, QString> status);
    static QByteArray generatePresence(QString type, QString from, QString to, QString id, QString show,
                                       QString priority, QString status);
    static QList<QByteArray> parseRequest(QByteArray data);
    static QByteArray generateMessage(QString type, QString from, QString to, QString body);
};

#endif // UTILS_H
