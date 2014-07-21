#ifndef VCARDMANAGER_H
#define VCARDMANAGER_H

#include <QObject>
#include "Error.h"
#include "UserManager.h"

class VCardManager : public QObject
{
    Q_OBJECT
public:
    explicit VCardManager(QObject *parent = 0, StorageManager *storageManager = 0);

    QByteArray vCardManagerReply(QDomDocument document, QString iqFrom);
    QByteArray generateVCardResult(QString from, QString to, QString id, QString vCard);
    QByteArray generateIQResult(QString to, QString id);

    QString getVCard(QString jid);
    bool updateVCard(QString jid, QString vCardInfos);
    bool vCardExist(QString jid);

signals:
    void sigSendReceiptRequest(QString to, QByteArray data);

public slots:

private:
    StorageManager *m_storageManager;
};

#endif // VCARDMANAGER_H
