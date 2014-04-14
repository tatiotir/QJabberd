#ifndef SERVICEDISCOVERYMANAGER_H
#define SERVICEDISCOVERYMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "Utils.h"
#include "Error.h"
#include "UserManager.h"

class ServiceDiscoveryManager : public QObject
{
    Q_OBJECT
public:
    explicit ServiceDiscoveryManager(UserManager *userManager = 0);

    QByteArray serviceDiscoveryManagerReply(QByteArray iqXML, QString iqFrom);

signals:
    void sigAccountAvailableResourceQuery(QString from, QString accountJid, QString id);
    void sigSendReceiptRequest(QString to, QByteArray data);
    void sigClientServiceDiscoveryQuery(QString to, QByteArray request);
    void sigClientServiceDiscoveryResponse(QString to, QByteArray response);

public slots:

private:
    QByteArray serviceDiscoveryManagerInfoQueryResult(QString from, QString to, QString id, QDomElement request);
    QByteArray serviceDiscoveryManagerItemsQueryResult(QString from, QString to, QString id,
                                                       QDomElement request);
    QByteArray serviceDiscoveryManagerInfoQueryResult(QString iqFrom, QDomElement iqElement);
    QByteArray serviceDiscoveryManagerItemsQueryResult(QString node, QString iqFrom);
    int getOfflineMessagesNumber(QString jid);
    QMultiHash<QString, QString> getOfflineMessageHeaders(QString jid);
    QMultiHash<QString, QString> getChatRoomList(QString room);

    UserManager *m_userManager;
};

#endif // SERVICEDISCOVERYMANAGER_H