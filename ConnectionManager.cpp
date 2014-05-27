#include "ConnectionManager.h"


/**
 * @brief ConnectionManager::ConnectionManager
 * @param parent
 * @param port
 * @param serverConfigMap
 */
ConnectionManager::ConnectionManager(QObject *parent, int port, QMap<QString, QVariant> *serverConfigMap) : QTcpServer(parent), m_port(port)
{
    m_serverConfigMap = serverConfigMap;
    m_listConnection = new QList<Connection *>();

    m_storageManager = new StorageManager(serverConfigMap->value("storageType").toString(),
                                          serverConfigMap->value("MySql").toMap());
    m_userManager = new UserManager(m_storageManager);
    m_offlineMessageManager = new OfflineMessageManager(m_storageManager);
    m_streamNegotiationManager = new StreamNegotiationManager(m_serverConfigMap, m_userManager);
    m_rosterManager = new RosterManager(m_storageManager);
    m_vCardManager = new VCardManager(m_storageManager);
    m_privateStorageManager = new PrivateStorageManager(m_storageManager);
    m_entityTimeManager = new EntityTimeManager();
    m_serviceDiscoveryManager = new ServiceDiscoveryManager(m_serverConfigMap, m_userManager);
    m_privacyListManager = new PrivacyListManager(m_storageManager);
    m_lastActivityManager= new LastActivityManager(m_userManager, m_rosterManager, m_storageManager);
    m_iqManager = new IQManager(m_serverConfigMap, m_userManager, m_privacyListManager, m_rosterManager, m_vCardManager,
                                m_lastActivityManager, m_entityTimeManager, m_privateStorageManager,
                                m_serviceDiscoveryManager, m_offlineMessageManager, m_streamNegotiationManager);
    m_messageManager = new MessageManager(m_userManager, m_privacyListManager);
    m_presenceManager = new PresenceManager( m_userManager, m_rosterManager, m_lastActivityManager,
                                            m_privacyListManager);

    m_streamManager = new StreamManager(this, m_storageManager, m_userManager, m_rosterManager,
                                        m_lastActivityManager);
    m_streamManager->start();

    // Connect SessionManager to the signal newConnection(Connection *)
    connect(this, SIGNAL(sigNewConnection(Connection*,IQManager*,PresenceManager*,
                                          MessageManager*,RosterManager*,StreamNegotiationManager*)),
            m_streamManager, SLOT(newConnection(Connection*,IQManager*,PresenceManager*,
                                                MessageManager*,RosterManager*,StreamNegotiationManager*)));

    connect(m_streamManager, SIGNAL(sigResourceBind(QString)), m_streamNegotiationManager,
            SLOT(resourceBind(QString)));
    connect(m_streamNegotiationManager, SIGNAL(sigHost(QString,QString)), m_streamManager,
            SLOT(streamHost(QString,QString)));

    connect(m_streamNegotiationManager, SIGNAL(sigStreamNegotiationError(QString)), m_streamManager,
            SLOT(streamNegotiationError(QString)));

    connect(m_iqManager, SIGNAL(sigNonSaslAuthentification(QString,QString,QString)),
            m_streamManager, SLOT(nonSaslAuthentification(QString,QString,QString)));

    connect(m_presenceManager, SIGNAL(sigPresenceBroadCast(QString,QByteArray)),
            m_streamManager, SLOT(presenceBroadCast(QString,QByteArray)));

    connect(m_presenceManager, SIGNAL(sigPresenceBroadCastFromContact(QString,QString)),
            m_streamManager, SLOT(presenceBroadCastFromContact(QString,QString)));

    connect(m_presenceManager, SIGNAL(sigPresencePriority(QString,int)), m_streamManager,
            SLOT(presencePriority(QString,int)));

    connect(m_presenceManager, SIGNAL(sigPresenceUnavailableBroadCast(QString,QString)), m_streamManager,
            SLOT(presenceUnavailableBroadCast(QString,QString)));

    connect(m_presenceManager, SIGNAL(sigPresenceProbeToContact(QString,QString,bool)), m_streamManager,
            SLOT(presenceProbeToContact(QString,QString,bool)));

    connect(m_presenceManager, SIGNAL(sigRosterPush(QString,QByteArray)), m_streamManager,
            SLOT(rosterPush(QString,QByteArray)));

    connect(m_presenceManager, SIGNAL(sigCurrentPresence(QString,QByteArray)), m_streamManager,
            SLOT(currentPresence(QString,QByteArray)));

    connect(m_presenceManager, SIGNAL(sigDirectedPresence(QString,QString,QByteArray)), m_streamManager,
            SLOT(directedPresence(QString,QString,QByteArray)));

    connect(m_messageManager, SIGNAL(sigNewChatMessage(QString,QByteArray)), m_streamManager,
            SLOT(sendMessage(QString,QByteArray)));

    connect(m_privacyListManager, SIGNAL(sigSetDefaultListName(QString,QString,QString,QString)), m_streamManager,
            SLOT(defaultListNameSetReply(QString,QString,QString,QString)));

    connect(m_iqManager, SIGNAL(sigStreamNegotiationError(QString)), m_streamManager,
            SLOT(streamNegotiationError(QString)));

    connect(m_privacyListManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));

    connect(m_iqManager, SIGNAL(sigResourceBinding(QString,QString,QString)), m_streamManager,
            SLOT(resourceBindingReply(QString,QString,QString)));

    connect(m_iqManager, SIGNAL(sigRosterPush(QString,QByteArray)), m_streamManager,
            SLOT(rosterPush(QString,QByteArray)));

    connect(m_iqManager, SIGNAL(sigLastActivityQuery(QString,QString,QString,QString)), m_streamManager,
            SLOT(lastActivityQuery(QString,QString,QString,QString)));

    connect(m_iqManager, SIGNAL(sigPresenceBroadCast(QString,QByteArray)), m_streamManager,
            SLOT(presenceBroadCast(QString,QByteArray)));

    connect(m_iqManager, SIGNAL(sigServerLastActivityQuery(QString,QString,QString)), m_streamManager,
            SLOT(serverLastActivityQuery(QString,QString,QString)));

    connect(m_iqManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));

    connect(m_serviceDiscoveryManager, SIGNAL(sigAccountAvailableResourceQuery(QString,QString,QString)),
            m_streamManager, SLOT(accountAvailableResourceQuery(QString,QString,QString)));

    connect(m_serviceDiscoveryManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));

    connect(m_serviceDiscoveryManager, SIGNAL(sigClientServiceDiscoveryQuery(QString,QByteArray)),
            m_streamManager, SLOT(clientServiceDiscoveryQuery(QString,QByteArray)));

    connect(m_lastActivityManager, SIGNAL(sigLastActivityQuery(QString,QString,QString,QString)),
            m_streamManager, SLOT(lastActivityQuery(QString,QString,QString,QString)));

    connect(m_lastActivityManager, SIGNAL(sigServerLastActivityQuery(QString,QString,QString)),
            m_streamManager, SLOT(serverLastActivityQuery(QString,QString,QString)));

    connect(m_lastActivityManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));

    connect(m_entityTimeManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));

    connect(m_vCardManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));
}

/**
 * This function start the connection manager
 * @brief ConnectionManager::startManage
 */
void ConnectionManager::startManage()
{
    if (listen(QHostAddress::Any, m_port))
    {
        qDebug() << "The server listen on port : " << m_port;
    }
    else
    {
        qDebug() << "The server can not start on port : " << m_port;
    }
}

/**
 * This function stop the connection manager
 * @brief ConnectionManager::stopManage
 */
void ConnectionManager::stopManage()
{
    this->close();
}

/**
 * This function is call when a new connection is available in the connection manager
 * We get the socket descriptor of the socket, create a new connection using the Connection class
 * and emit the signal "sigNewConnection to the stream manager.
 *
 * @brief ConnectionManager::incomingConnection
 * @param socketDescriptor
 */
void ConnectionManager::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New client connected on server";

    // We create new socket connection but we don't start tls v1 encryption at this time.
    Connection *connection = new Connection(this);
    connection->setSocketDescriptor(socketDescriptor);

    m_listConnection->append(connection);
    emit sigNewConnection(connection, m_iqManager, m_presenceManager, m_messageManager,
                          m_rosterManager, m_streamNegotiationManager);
}

/**
 * @brief ConnectionManager::deconnection
 */
void ConnectionManager::deconnection()
{

}

/**
 * @brief ConnectionManager::getPort
 * @return port
 */
int ConnectionManager::getPort()
{
    return m_port;
}
