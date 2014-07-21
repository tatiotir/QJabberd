#ifndef BLOCKINGCOMMANDMANAGER_H
#define BLOCKINGCOMMANDMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "Error.h"
#include "RosterManager.h"

class BlockingCommandManager : public QObject
{
    Q_OBJECT
public:
    explicit BlockingCommandManager(QObject *parent = 0, StorageManager *storageManager = 0,
                                    RosterManager *rosterManager = 0);

    QByteArray blockingCommandManagerReply(QDomDocument document, QString iqFrom);
    QByteArray blockingCommandManagerResult(QString id, QList<QString> blocklist);
    static QDomDocument generateBlockPush(QString to, QString id, QList<QString> items);
    static QDomDocument generateUnblockPush(QString to, QString id, QList<QString> items);
    QByteArray isBlocked(QString from, QString to, QString stanzaType);

signals:
    void sigUnavailablePresenceBroadCast(QString to, QString from);
    void sigPresenceBroadCastFromContact(QString to, QString from);
    void sigBlockPush(QString to, QList<QString> items);
    void sigUnblockPush(QString to, QList<QString> items);

public slots:

private:
    QByteArray generateIQResult(QString to, QString id);
    bool addUserBlockListItems(QString jid, QList<QString> items);
    bool deleteUserBlockListItems(QString jid, QList<QString> items);
    bool emptyUserBlockList(QString jid);
    QList<QString> getUserBlockList(QString jid);

    StorageManager *m_storageManager;
    RosterManager *m_rosterManager;
};

#endif // BLOCKINGCOMMANDMANAGER_H
