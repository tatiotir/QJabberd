#ifndef PRIVATESTORAGEMANAGER_H
#define PRIVATESTORAGEMANAGER_H

#include <QObject>
#include <QDomDocument>
#include "Utils.h"
#include "Error.h"
#include "StorageManager.h"
#include "MetaContact.h"

class PrivateStorageManager : public QObject
{
    Q_OBJECT
public:
    explicit PrivateStorageManager(StorageManager *storageManager = 0);

    QByteArray privateStorageManagerReply(QDomDocument document, QString from);
    QByteArray privateStorageManagerResult(QByteArray data, QString id);
    QByteArray privateStorageManagerResult(QList<MetaContact> metacontactList, QString id);

signals:
    void sigSendReceiptRequest(QString to, QByteArray data);

public slots:

private:
    bool storePrivateData(QString jid, QMultiHash<QString, QString> nodeMap);
    bool storePrivateData(QString jid, QList<MetaContact> metaContactList);
    QByteArray getPrivateData(QString jid, QString node);
    QList<MetaContact> getPrivateData(QString jid);

    StorageManager *m_storageManager;
};

#endif // PRIVATESTORAGEMANAGER_H
