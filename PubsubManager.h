#ifndef PUBSUBMANAGER_H
#define PUBSUBMANAGER_H

#include <QObject>
#include <QDomDocument>
#include "RosterManager.h"
#include "Utils.h"
#include "Error.h"

class PubsubManager : public QObject
{
    Q_OBJECT
public:
    explicit PubsubManager(RosterManager *rosterManager = 0, StorageManager *storageManager = 0);

    QByteArray pubsubManagerReply(QDomDocument document, QString iqFrom);

signals:

public slots:

private:
    QByteArray pubsubSubscribeNotification(QString from, QString to, QString id, QString node, QString jid,
                                           QString subscription);
    QByteArray pubsubUnconfiguredNotification(QString from, QString to, QString id, QString node, QString jid, bool required);
    //QByteArray pubsubItemPush(QString node, QString itemId);


    bool subscribeToNode(QString node, QString jid);
    QString nodeAccessModel(QString node);
    QString nodeOwner(QString node);
    QStringList authorizedRosterGroups(QString node);
    QStringList nodeWhiteList(QString node);
    QStringList nodeCustomerDatabase(QString node);
    QString nodeUserSubscription(QString node, QString jid);
    QString nodeUserAffiliation(QString node, QString jid);
    bool allowSubscription(QString node);
    bool nodeExist(QString node);
    bool configurationRequired(QString node);

    RosterManager *m_rosterManager;
    StorageManager *m_storageManager;

};

#endif // PUBSUBMANAGER_H
