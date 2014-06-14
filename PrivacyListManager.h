#ifndef PRIVACYLISTMANAGER_H
#define PRIVACYLISTMANAGER_H

#include <QObject>
#include "Error.h"
#include "UserManager.h"
#include "RosterManager.h"

class PrivacyListManager : public QObject
{
    Q_OBJECT
public:
    explicit PrivacyListManager(StorageManager *storageManager = 0, RosterManager *rosterManager = 0);

signals:
    void sigSetDefaultListName(QString jid, QString to, QString defaultListName, QString id);
    void sigPrivacyListPush(QString to, QDomDocument document);
    void sigSendReceiptRequest(QString to, QByteArray data);

public slots:
    QByteArray privacyListReply(QDomDocument document, QString iqFrom);
    QByteArray generatePrivacyListResult(QString to, QString id, QString privacyListName, QList<PrivacyListItem> items);
    QByteArray generateIQResult(QString to, QString id);
    QDomDocument generatePrivacyPush(QString to, QString privacyListName, QString id);
    QByteArray isBlocked(QString from, QString to, QString stanzaType);
    QByteArray isBlocked(QString from, QString to, QList<PrivacyListItem> privacyListAllowItems, QList<PrivacyListItem> privacyListDenyItems);
    QList<PrivacyListItem> getPrivacyListItems(QString jid, QString privacyListName, QString stanzaType,
                                               QString action);
    QString getDefaultPrivacyList(QString jid);
    QString getActivePrivacyList(QString jid);

private:
    QList<PrivacyListItem> getPrivacyList(QString jid, QString privacyListName);
    QStringList getPrivacyListNames(QString jid);
    bool setDefaultPrivacyList(QString jid, QString defaultList);
    bool setActivePrivacyList(QString jid, QString activeList);
    bool addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items);
    bool deletePrivacyList(QString jid, QString privacyListName);
    bool privacyListExist(QString jid, QString privacyListName);
    QByteArray privacyListNames(QString from, QString to, QString id, QStringList privacyListNames, QString activeListName, QString defaultListName);

    StorageManager *m_storageManager;
    RosterManager *m_rosterMananager;
};

#endif // PRIVACYLISTMANAGER_H
