#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QtNetwork/QTcpServer>
#include "StreamManager.h"
#include "StorageManager.h"

class ConnectionManager : public QTcpServer
{
    Q_OBJECT
public:
    ConnectionManager(QObject *parent = 0, int port = 5222, QMap<QString, QVariant> *serverConfigMap = 0);

public slots:
    int getPort();
    void startManage();
    void stopManage();
    void deconnection();


signals:
    void sigNewConnection(Connection *connection, IQManager *iqManager,
                          PresenceManager *presenceManager, MessageManager *messageManager,
                          RosterManager *rosterManager, StreamNegotiationManager *streamNegotiationManager);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    int m_port;
    QList<Connection *> *m_listConnection;
    StreamManager *m_streamManager;
    UserManager *m_userManager;
    OfflineMessageManager *m_offlineMessageManager;
    IQManager *m_iqManager;
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
    QString m_storageType;
    StorageManager *m_storageManager;
    QMap<QString, QVariant> *m_serverConfigMap;
};

#endif // CONNECTIONMANAGER_H
