#ifndef PRIVACYLISTMANAGER_H
#define PRIVACYLISTMANAGER_H

#include <QObject>
#include "Error.h"
#include "UserManager.h"

class PrivacyListManager : public QObject
{
    Q_OBJECT
public:
    explicit PrivacyListManager(StorageManager *storageManager = 0);

signals:
    void sigSetDefaultListName(QString jid, QString to, QString defaultListName, QString id);
    void sigPrivacyListPush(QString to, QByteArray data);
    void sigSendReceiptRequest(QString to, QByteArray data);

public slots:
    QByteArray privacyListReply(QDomDocument document, QString iqFrom);
    QByteArray generatePrivacyListResult(QString to, QString id, QString privacyListName, QList<PrivacyListItem> items);
    QByteArray generateIQResult(QString to, QString id);
    QByteArray generatePrivacyPush(QString to, QString privacyListName, QString id);

    void setPrivacyActiveList(QString activeList);
    QString getPrivacyActiveList();
    void setPrivacyDefaultList(QString activeList);
    QString getPrivacyDefaultList();

private:
    QList<PrivacyListItem> getPrivacyList(QString jid, QString privacyListName);
    bool addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items);
    bool deletePrivacyList(QString jid, QString privacyListName);
    QByteArray privacyListNames(QString from, QString to, QString id);

    StorageManager *m_storageManager;
    QString m_privacyActiveListName;
    QString m_privacyDefaultListName;
};

#endif // PRIVACYLISTMANAGER_H
