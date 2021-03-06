#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "Stream.h"
#include "User.h"

class StreamManager : public QObject
{
    Q_OBJECT
public:
    StreamManager(QObject *parent = 0, QJsonObject *serverConfiguration = 0, StorageManager *storageManager = 0,
                  UserManager *userManager = 0, RosterManager *rosterManager = 0,
                  LastActivityManager *lastActivityManager = 0);

public slots:
    void newConnection(Connection *connection, IqManager *iqManager,
                       PresenceManager *presenceManager, MessageManager *messageManager,
                       RosterManager *rosterManager, StreamNegotiationManager *streamNegotiationManager,
                       BlockingCommandManager *blockingCmdManager);
    void resourceBindingReply(QString streamId, QString fullJid, QString id);
    void streamNegotiationError(QString streamId);
    void nonSaslAuthentification(QString streamId, QString fullJid, QString id);
    bool saveOfflineMessage(QString from, QString to, QString type, QList<QPair<QString, QString> > bodyPairList,
                            QString stamp);
    QMultiHash<QString, QByteArray> getAllOfflineMessage(QString jid);
    QList<QVariant> getOfflinePresenceSubscription(QString jid);
    void saveOfflinePresenceSubscription(QString from, QString to, QByteArray presence, QString presenceType);
    //void deleteOfflinePresenceSubscribe(QString from, QString to);
    void sendOfflineMessage(QString to);
    void sendReceiptRequest(QString to, QByteArray data);
    void slotSendReceiptRequest(QString to, QByteArray data);
    void saveStream(QString fullJid, Stream *stream);
    void closeStream(QString fullJid);
    void streamHost(QString streamId, QString host);
    void rosterPush(QString to, QDomDocument document);
    void presenceBroadCast(QString to, QDomDocument document);
    void presenceBroadCastFromContact(QString to, QString contactJid);
    void presenceProbeReply(QString to, QString from, bool directedPresenceProbe);
    void presenceProbeToContact(QString to, QString from, bool directedPresenceProbe);
    void sendMessage(QString to, QDomDocument document);
    void presencePriority(QString fullJid, int priority);
    void defaultListNameSetReply(QString jid, QString to, QString defaultListName, QString id);
    void sendUndeliveredPresence(QString to);
    void presenceUnavailableBroadCast(QString to, QString from);
    void lastActivityQuery(QString from, QString to, QString id, QString lastStatus);
    void offlineUser(QString fullJid);
    void serverLastActivityQuery(QString from, QString to, QString id);
    void currentPresence(QString fullJid, QByteArray presenceData);
    void directedPresence(QString from, QString to, QByteArray data);
    void accountAvailableResourceQuery(QString from, QString accountJid, QString id);
    void enableStreamManagement(QString fullJid, QString smId);
    void inboundStanzaReceived(QString from);
    void queryInboundStanzaReceived(QString from);
    void acknowledgeReceiptServerStanza(QString from, int h);
    void resumeStream(Connection *connection, QString prevId, int h);
    void clientServiceDiscoveryQuery(QString to, QByteArray request);
    void clientServiceDiscoveryResponse(QString to, QByteArray response);
    void requestRedirection(QString to, QDomDocument document);
    void blockPush(QString to, QList<QString> items);
    void unblockPush(QString to, QList<QString> items);
    void mucPresenceBroadCast(QString to, QDomDocument document);
    void roomHistory(QString to, QList<QDomDocument> messageList);
    void roomSubject(QString to, QByteArray subjectMessage);
    void groupchatMessage(QString to, QDomDocument document);
    void directMucInvitation(QString to, QDomDocument document);
    void pubsubNotification(QString to, QByteArray notification);

signals:
    void sigPresenceSubscribedSended();
    void sigResourceBind(QString streamId);
    //void sigOfflineUser(QString jid);
    //void sigSetDefaultListName(QString jid, QString to, QString defaultListName, QString id);

protected:
    void run();

private:
    bool saveStreamData(QString smId, QByteArray data); // Save the data send to the client before send him the </r> request
    bool deleteStreamData(QString smId, int h);
    bool saveStreamPresencePriority(QString smId, int presencePriority);
    bool saveStreamPresenceStanza(QString smId, QByteArray presenceData);
    QList<QByteArray> getClientUnhandleStanza(QString smId);

    QJsonObject *m_serverConfiguration;
    QTime m_serverTime;
    StorageManager *m_storageManager;
    UserManager *m_userManager;
    LastActivityManager *m_lastActivityManager;
    RosterManager *m_rosterManager;
    QMultiHash<QString, User* > *m_userMap;
    QMultiHash<QString, Stream* > *m_notNegotiatedStream;
    QStringList *m_resourceDefaultList; // this list contain the resource who has set a default list
};

#endif // SESSIONMANAGER_H
