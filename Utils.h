#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QStringList>
#include <QTime>
#include <QSet>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QDomElement>
#include <QDomDocument>
#include <QDebug>

class Utils
{
public:
    Utils();

    static QByteArray generateHttpResponseHeader(int contentLength);
    static QMap<QByteArray, QByteArray> parseHttpRequest(QByteArray postData);
    static QByteArray digestCalculator(QString id, QString password);
    static QString generateResource();
    static QString generateId();
    static QString getBareJid(QString fullJid);
    static QString getResource(QString fullJid);
    static QString getUsername(QString fullJid);
    static QString getHost(QString fullJid);
    static QDomDocument generateRosterPush(QString to, QString id, QString jid, QString name,
                                            QString subscription, QString ask, bool approved, QSet<QString> groupList);
    static QDomDocument generateBlockPush(QString to, QString id, QList<QString> items);
    static QDomDocument generateUnblockPush(QString to, QString id, QList<QString> items);
    static QDomDocument generatePresence(QString type, QString from, QString to, QString id, QString show,
                                       QString priority, QMultiHash<QString, QString> status);
    static QDomDocument generatePresence(QString type, QString from, QString to, QString id, QString show,
                                       QString priority, QString status);
    static QDomDocument generatePresence(QString type, QString from, QString to, QString id, QString affiliation,
                                         QString role, QString jid, QString nick, QList<int> statusCodes, QString status, QString show);
    static QList<QByteArray> parseRequest(QByteArray data);
    static QDomDocument generateMucInvitationMessage(QString from, QString to, QString id, QString inviterJid,
                                               QString roomPassword, QString reason);
    static QDomDocument generateMucNotificationMessage(QString type, QString from, QString to, QString id, QList<int> statusCode);
    static int affiliationIntValue(QString affiliation);
    static QString hmac(QByteArray key, QByteArray baseString);
    static QString XOR(QByteArray a, QByteArray b);
    static QByteArray generateIQResult(QString from, QString to, QString id);
};

#endif // UTILS_H
