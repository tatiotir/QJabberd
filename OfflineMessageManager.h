#ifndef OFFLINEMESSAGEMANAGER_H
#define OFFLINEMESSAGEMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "StorageManager.h"

class OfflineMessageManager : public QObject
{
    Q_OBJECT
public:
    explicit OfflineMessageManager(StorageManager *storageManager = 0);

    QByteArray offlineMessageManagerReply(QByteArray iqXML, QString iqFrom);
    QByteArray offlineMessageManagerResult(QString to, QString id, QByteArray messageList);

signals:

public slots:

private:
    QMultiHash<QString, QByteArray> getOfflineMessageFrom(QString jid, QString from);
    QByteArray getOfflineMessage(QString jid, QString stamp);
    QMultiHash<QString, QByteArray> getAllOfflineMessage(QString jid);
    QMultiHash<QString, QString> getOfflineMessageHeaders(QString jid);
    void deleteOfflineMessage(QString jid, QString key);
    void deleteAllOfflineMessage(QString jid);


    StorageManager *m_storageManager;

};

#endif // OFFLINEMESSAGEMANAGER_H
