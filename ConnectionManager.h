#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QtNetwork/QTcpServer>
#include "StreamManager.h"
#include "StorageManager.h"

class ConnectionManager : public QTcpServer
{
    Q_OBJECT
public:
    ConnectionManager(QObject *parent = 0, int port = 5222, QJsonObject *serverConfiguration = 0);

public slots:
    int getPort();
    void startManage();
    void stopManage();
    void deconnection();

signals:
    void sigNewConnection(Connection *connection, IqManager *iqManager,
                          PresenceManager *presenceManager, MessageManager *messageManager,
                          RosterManager *rosterManager, StreamNegotiationManager *streamNegotiationManager,
                          BlockingCommandManager *blockingCmdManager);
    void sigConnectionManagerBoshSessionInitiationReply(QString sid, QDomDocument document);
    void sigConnectionManagerBoshRequestReply(QString sid, QList<QDomDocument> listDocument);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    void createNetworkProxy(QJsonObject *serverConfiguration);
    QJsonObject *m_serverConfiguration;
    QList<QNetworkProxy> m_networkProxyList;
    int m_port;
    QList<Connection *> *m_listConnection;
    StreamManager *m_streamManager;
    UserManager *m_userManager;
    OfflineMessageManager *m_offlineMessageManager;
    IqManager *m_iqManager;
    PrivateStorageManager *m_privateStorageManager;
    PresenceManager *m_presenceManager;
    MessageManager *m_messageManager;
    EntityTimeManager *m_entityTimeManager;
    LastActivityManager *m_lastActivityManager;
    VCardManager *m_vCardManager;
    RosterManager *m_rosterManager;
    PrivacyListManager *m_privacyListManager;
    ServiceDiscoveryManager *m_serviceDiscoveryManager;
    StreamNegotiationManager *m_streamNegotiationManager;
    StorageManager *m_storageManager;
    BlockingCommandManager *m_blockingCmdManager;
    MucManager *m_mucManager;
    ByteStreamsManager *m_bytestreamsManager;
    PubsubManager *m_pubsubManager;
};

#endif // CONNECTIONMANAGER_H
