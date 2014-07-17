#include "ConnectionManager.h"


/*!
 * \brief ConnectionManager::ConnectionManager
 * \param parent
 * \param port
 * \param serverConfigMap
 */
ConnectionManager::ConnectionManager(QObject *parent, int port, QJsonObject *serverConfiguration) : QTcpServer(parent), m_port(port)
{
    // Create network proxy
    createNetworkProxy(serverConfiguration);
    m_listConnection = new QList<Connection *>();

    m_storageManager = new StorageManager(serverConfiguration->value("storageType").toString(),
                                          serverConfiguration->value(serverConfiguration->value("storageType").toString()).toObject());
    m_userManager = new UserManager(m_storageManager);
    m_mucManager = new MucManager(m_storageManager);
    m_bytestreamsManager = new ByteStreamsManager(serverConfiguration);
    m_offlineMessageManager = new OfflineMessageManager(m_storageManager);
    m_streamNegotiationManager = new StreamNegotiationManager(serverConfiguration, m_userManager);
    m_rosterManager = new RosterManager(m_storageManager);
    m_blockingCmdManager = new BlockingCommandManager(m_storageManager, m_rosterManager);
    m_vCardManager = new VCardManager(m_storageManager);
    m_privateStorageManager = new PrivateStorageManager(m_storageManager);
    m_entityTimeManager = new EntityTimeManager();
    m_serviceDiscoveryManager = new ServiceDiscoveryManager(serverConfiguration, m_userManager, m_mucManager);
    m_privacyListManager = new PrivacyListManager(m_storageManager, m_rosterManager);
    m_lastActivityManager= new LastActivityManager(m_userManager, m_rosterManager, m_storageManager);
    m_iqManager = new IqManager(serverConfiguration, m_userManager, m_privacyListManager, m_rosterManager,
                                m_vCardManager, m_lastActivityManager, m_entityTimeManager,
                                m_privateStorageManager, m_serviceDiscoveryManager, m_offlineMessageManager,
                                m_streamNegotiationManager, m_blockingCmdManager, m_mucManager,
                                m_bytestreamsManager);
    m_messageManager = new MessageManager(serverConfiguration, m_userManager, m_privacyListManager, m_mucManager,
                                          m_blockingCmdManager);
    m_presenceManager = new PresenceManager( m_userManager, m_rosterManager, m_lastActivityManager,
                                            m_privacyListManager, m_mucManager, m_blockingCmdManager);

    m_streamManager = new StreamManager(this, m_storageManager, m_userManager, m_rosterManager,
                                        m_lastActivityManager);
    m_streamManager->start();

    // Connect SessionManager to the signal newConnection(Connection *)
    connect(this, SIGNAL(sigNewConnection(Connection*,IqManager*,PresenceManager*,MessageManager*,
                                          RosterManager*,StreamNegotiationManager*,BlockingCommandManager*)),
            m_streamManager, SLOT(newConnection(Connection*,IqManager*,PresenceManager*,MessageManager*,
                                                RosterManager*,StreamNegotiationManager*,
                                                BlockingCommandManager*)));

    connect(m_streamManager, SIGNAL(sigResourceBind(QString)), m_streamNegotiationManager,
            SLOT(resourceBind(QString)));

    connect(m_streamNegotiationManager, SIGNAL(sigHost(QString,QString)), m_streamManager,
            SLOT(streamHost(QString,QString)));

    connect(m_streamNegotiationManager, SIGNAL(sigStreamNegotiationError(QString)), m_streamManager,
            SLOT(streamNegotiationError(QString)));

    connect(m_iqManager, SIGNAL(sigNonSaslAuthentification(QString,QString,QString)),
            m_streamManager, SLOT(nonSaslAuthentification(QString,QString,QString)));

    connect(m_iqManager, SIGNAL(sigApplicationReply(QString,QDomDocument)), m_streamManager,
            SLOT(requestRedirection(QString,QDomDocument)));

    connect(m_iqManager, SIGNAL(sigApplicationRequest(QString,QDomDocument)), m_streamManager,
            SLOT(requestRedirection(QString,QDomDocument)));

    connect(m_iqManager, SIGNAL(sigIqAvatarQuery(QString,QDomDocument)), m_streamManager,
            SLOT(requestRedirection(QString,QDomDocument)));

    connect(m_presenceManager, SIGNAL(sigPresenceBroadCast(QString,QDomDocument)),
            m_streamManager, SLOT(presenceBroadCast(QString,QDomDocument)));

    connect(m_presenceManager, SIGNAL(sigPresenceBroadCastFromContact(QString,QString)),
            m_streamManager, SLOT(presenceBroadCastFromContact(QString,QString)));

    connect(m_presenceManager, SIGNAL(sigPresencePriority(QString,int)), m_streamManager,
            SLOT(presencePriority(QString,int)));

    connect(m_presenceManager, SIGNAL(sigPresenceUnavailableBroadCast(QString,QString)), m_streamManager,
            SLOT(presenceUnavailableBroadCast(QString,QString)));

    connect(m_presenceManager, SIGNAL(sigPresenceProbeToContact(QString,QString,bool)), m_streamManager,
            SLOT(presenceProbeToContact(QString,QString,bool)));

    connect(m_presenceManager, SIGNAL(sigRosterPush(QString,QDomDocument)), m_streamManager,
            SLOT(rosterPush(QString,QDomDocument)));

    connect(m_presenceManager, SIGNAL(sigCurrentPresence(QString,QByteArray)), m_streamManager,
            SLOT(currentPresence(QString,QByteArray)));

    connect(m_presenceManager, SIGNAL(sigDirectedPresence(QString,QString,QByteArray)), m_streamManager,
            SLOT(directedPresence(QString,QString,QByteArray)));

    connect(m_presenceManager, SIGNAL(sigMucPresenceBroadCast(QString,QDomDocument)), m_streamManager,
            SLOT(mucPresenceBroadCast(QString,QDomDocument)));

    connect(m_presenceManager, SIGNAL(sigRoomHistory(QString,QList<QDomDocument>)), m_streamManager,
            SLOT(roomHistory(QString,QList<QDomDocument>)));

    connect(m_presenceManager, SIGNAL(sigRoomSubject(QString,QByteArray)), m_streamManager,
            SLOT(roomSubject(QString,QByteArray)));

    connect(m_messageManager, SIGNAL(sigNewChatMessage(QString,QDomDocument)), m_streamManager,
            SLOT(sendMessage(QString,QDomDocument)));

    connect(m_messageManager, SIGNAL(sigDirectMucInvitation(QString,QDomDocument)), m_streamManager,
            SLOT(directMucInvitation(QString,QDomDocument)));

    connect(m_messageManager, SIGNAL(sigGroupchatMessage(QString,QDomDocument)), m_streamManager,
            SLOT(groupchatMessage(QString,QDomDocument)));

    connect(m_messageManager, SIGNAL(sigMucPresenceBroadCast(QString,QDomDocument)), m_streamManager,
            SLOT(mucPresenceBroadCast(QString,QDomDocument)));

    connect(m_messageManager, SIGNAL(sigApplicationRequest(QString,QDomDocument)), m_streamManager,
            SLOT(requestRedirection(QString,QDomDocument)));

    connect(m_privacyListManager, SIGNAL(sigSetDefaultListName(QString,QString,QString,QString)), m_streamManager,
            SLOT(defaultListNameSetReply(QString,QString,QString,QString)));

    connect(m_privacyListManager, SIGNAL(sigPrivacyListPush(QString,QDomDocument)), m_streamManager,
            SLOT(requestRedirection(QString,QDomDocument)));

    connect(m_iqManager, SIGNAL(sigStreamNegotiationError(QString)), m_streamManager,
            SLOT(streamNegotiationError(QString)));

    connect(m_privacyListManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));

    connect(m_iqManager, SIGNAL(sigResourceBinding(QString,QString,QString)), m_streamManager,
            SLOT(resourceBindingReply(QString,QString,QString)));

    connect(m_iqManager, SIGNAL(sigRosterPush(QString,QDomDocument)), m_streamManager,
            SLOT(rosterPush(QString,QDomDocument)));

//    connect(m_iqManager, SIGNAL(sigLastActivityQuery(QString,QString,QString,QString)), m_streamManager,
//            SLOT(lastActivityQuery(QString,QString,QString,QString)));

    connect(m_iqManager, SIGNAL(sigPresenceBroadCast(QString,QDomDocument)), m_streamManager,
            SLOT(presenceBroadCast(QString,QDomDocument)));

//    connect(m_iqManager, SIGNAL(sigServerLastActivityQuery(QString,QString,QString)), m_streamManager,
//            SLOT(serverLastActivityQuery(QString,QString,QString)));

    connect(m_iqManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));

    connect(m_iqManager, SIGNAL(sigGroupchatMessage(QString,QDomDocument)), m_streamManager,
            SLOT(groupchatMessage(QString,QDomDocument)));

    connect(m_iqManager, SIGNAL(sigMucPresenceBroadCast(QString,QDomDocument)), m_streamManager,
            SLOT(mucPresenceBroadCast(QString,QDomDocument)));

    connect(m_serviceDiscoveryManager, SIGNAL(sigAccountAvailableResourceQuery(QString,QString,QString)),
            m_streamManager, SLOT(accountAvailableResourceQuery(QString,QString,QString)));

    connect(m_serviceDiscoveryManager, SIGNAL(sigSendReceiptRequest(QString,QByteArray)), m_streamManager,
            SLOT(slotSendReceiptRequest(QString,QByteArray)));

    connect(m_serviceDiscoveryManager, SIGNAL(sigClientServiceDiscoveryQuery(QString,QByteArray)),
            m_streamManager, SLOT(clientServiceDiscoveryQuery(QString,QByteArray)));

    connect(m_serviceDiscoveryManager, SIGNAL(sigClientServiceDiscoveryResponse(QString,QByteArray)),
            m_streamManager, SLOT(clientServiceDiscoveryResponse(QString,QByteArray)));

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

    connect(m_blockingCmdManager, SIGNAL(sigBlockPush(QString,QList<QString>)), m_streamManager,
            SLOT(blockPush(QString,QList<QString>)));

    connect(m_blockingCmdManager, SIGNAL(sigUnblockPush(QString,QList<QString>)), m_streamManager,
            SLOT(unblockPush(QString,QList<QString>)));

    connect(m_blockingCmdManager, SIGNAL(sigUnavailablePresenceBroadCast(QString,QString)), m_streamManager,
            SLOT(presenceUnavailableBroadCast(QString,QString)));

    connect(m_blockingCmdManager, SIGNAL(sigPresenceBroadCastFromContact(QString,QString)), m_streamManager,
            SLOT(presenceBroadCastFromContact(QString,QString)));

}

/*!
 * \brief The ConnectionManager::startManage function start the connection manager
 */
void ConnectionManager::startManage()
{
    if (listen(QHostAddress::Any, m_port))
    {
        qDebug() << "QJabberd listen on port : " << m_port;
    }
    else
    {
        qDebug() << "QJabberd can not start on port : " << m_port;
    }
}

/*!
 * \brief The ConnectionManager::stopManage method stop the connection manager
 */
void ConnectionManager::stopManage()
{
    this->close();
}

/*!
 * \brief The ConnectionManager::incomingConnection method is call when a new connection is available in the connection manager
 * We get the socket descriptor of the socket, create a new connection using the Connection class
 * and emit the signal "sigNewConnection to the stream manager.
 * \param socketDescriptor
 */
void ConnectionManager::incomingConnection(qintptr socketDescriptor)
{
    //qDebug() << "New client connected on server";

    // We create new socket connection but we don't start tls v1 encryption at this time.
    Connection *connection = new Connection(this);
    connection->setSocketDescriptor(socketDescriptor);

    m_listConnection->append(connection);
    emit sigNewConnection(connection,
                          m_iqManager,
                          m_presenceManager,
                          m_messageManager,
                          m_rosterManager,
                          m_streamNegotiationManager,
                          m_blockingCmdManager);
}

void ConnectionManager::createNetworkProxy(QJsonObject *serverConfiguration)
{
    foreach (QString host, serverConfiguration->value("services").toVariant().toMap().keys())
    {
        QJsonArray hostServiceList = serverConfiguration->value("services").toObject().value(host).toArray();
        for (int i = 0; i < hostServiceList.count(); ++i)
        {
            QJsonObject proxyService = hostServiceList[i].toObject();
            if (proxyService.value("type").toString() == "proxy")
            {
                QNetworkProxy networkProxy;
                networkProxy.setHostName(proxyService.value("host").toString());
                networkProxy.setPort(proxyService.value("port").toInt());
                networkProxy.setUser(proxyService.value("username").toString());
                networkProxy.setPassword(proxyService.value("password").toString());

                setProxy(networkProxy);
                m_networkProxyList.append(networkProxy);
            }
        }
    }
}

/*!
 * \brief ConnectionManager::deconnection
 */
void ConnectionManager::deconnection()
{

}

/*!
 * \brief ConnectionManager::getPort
 * @return port
 */
int ConnectionManager::getPort()
{
    return m_port;
}

void ConnectionManager::boshSessionInitiation(QString sid, QString host)
{
    StreamNegotiationData *strData = new StreamNegotiationData();
    strData->setHost(host);
    m_streamNegotiationManager->setStreamNegotiationData(sid, strData);

    QDomDocument document;
    document.setContent(m_streamNegotiationManager->secondFeatures());
    emit sigConnectionManagerBoshSessionInitiationReply(sid, document);
}

void ConnectionManager::boshSessionRequest(QString sid, QString fullJid, QString host,
                                           QList<QDomDocument> requests)
{
    QList<QDomDocument> listResponse;
    foreach (QDomDocument document, requests)
    {
        //qDebug() << "Connection manager request from Bosh Server : " << document.toString();
        if (document.documentElement().tagName() == "iq")
        {
            QByteArray answer = m_iqManager->parseIQ(document, fullJid, host, sid);

            QDomDocument replyDocument;
            replyDocument.setContent(answer);
            listResponse << replyDocument;
        }
        else if (document.documentElement().tagName() == "presence")
        {
            QByteArray answer = m_presenceManager->parsePresence(document, fullJid);

            QDomDocument replyDocument;
            replyDocument.setContent(answer);
            listResponse << replyDocument;
        }
        else if (document.documentElement().tagName() == "message")
        {
            QByteArray answer = m_messageManager->parseMessage(document, fullJid);

            QDomDocument replyDocument;
            replyDocument.setContent(answer);
            listResponse << replyDocument;
        }
        else
        {
            QByteArray answer = m_streamNegotiationManager->reply(document, sid);

            QDomDocument replyDocument;
            replyDocument.setContent(answer);
            listResponse << replyDocument;
        }
    }
    emit sigConnectionManagerBoshRequestReply(sid, listResponse);
}
