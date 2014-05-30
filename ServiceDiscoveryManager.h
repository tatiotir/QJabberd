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

class ServiceDiscoveryManager : public QObject
{
    Q_OBJECT
public:
    explicit ServiceDiscoveryManager(QMap<QString, QVariant> *serverConfigMap = 0,
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
    QByteArray serviceDiscoveryManagerItemsQueryResult(QString node, QString iqFrom);
    int getOfflineMessagesNumber(QString jid);
    QMultiHash<QString, QString> getOfflineMessageHeaders(QString jid);

    UserManager *m_userManager;
    MucManager *m_mucManager;
    QMap<QString, QVariant> *m_serverConfigMap;
};

#endif // SERVICEDISCOVERYMANAGER_H
