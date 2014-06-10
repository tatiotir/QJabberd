#ifndef SERVICEDISCOVERYMANAGER_H
#define SERVICEDISCOVERYMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "Utils.h"
#include "Error.h"
#include "UserManager.h"
#include "StreamNegotiationManager.h"
#include "MucManager.h"
#include "DataFormManager.h"

class ServiceDiscoveryManager : public QObject
{
    Q_OBJECT
public:
    explicit ServiceDiscoveryManager(QJsonObject *serverConfiguration = 0,
                                     UserManager *userManager = 0, MucManager *mucManager = 0);

    QByteArray serviceDiscoveryManagerReply(QDomDocument document, QString iqFrom);

signals:
    void sigAccountAvailableResourceQuery(QString from, QString accountJid, QString id);
    void sigSendReceiptRequest(QString to, QByteArray data);
    void sigClientServiceDiscoveryQuery(QString to, QByteArray request);
    void sigClientServiceDiscoveryResponse(QString to, QByteArray response);

public slots:

private:
    QByteArray serviceDiscoveryManagerInfoQueryResult(QString from, QString to, QString id,
                                                      QDomDocument document);
    QByteArray serviceDiscoveryManagerItemsQueryResult(QString from, QString to, QString id,
                                                       QDomDocument document);
    QByteArray serviceDiscoveryManagerInfoQueryResult(QString iqFrom, QString node, QDomDocument document);
    QByteArray serviceDiscoveryManagerItemsQueryResult(QString node, QString iqFrom, QDomDocument document);
    int getOfflineMessagesNumber(QString jid);
    QJsonObject chatServiceExist(QString serviceJid);
    QMultiHash<QString, QString> getOfflineMessageHeaders(QString jid);

    UserManager *m_userManager;
    MucManager *m_mucManager;
    QJsonObject *m_serverConfiguration;
};

#endif // SERVICEDISCOVERYMANAGER_H
