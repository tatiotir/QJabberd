#ifndef IQMANAGER_H
#define IQMANAGER_H

#include <QDebug>
#include <QString>
#include <QDomElement>
#include <QDomNodeList>
#include "UserManager.h"
#include "DataFormManager.h"
#include "Error.h"
#include "ServiceDiscoveryManager.h"
#include "PrivacyListManager.h"
#include "VcardManager.h"
#include "PresenceManager.h"
#include "LastActivityManager.h"
#include "EntityTimeManager.h"
#include "RosterManager.h"
#include "PrivateStorageManager.h"
#include "OfflineMessageManager.h"
#include "StreamNegotiationManager.h"
#include "BlockingCommandManager.h"
#include "MucManager.h"
#include "ByteStreamsManager.h"
#include "PubsubManager.h"

class IqManager : public QObject
{
    Q_OBJECT
public:
    IqManager(QObject *parent = 0, QJsonObject *serverConfiguration = 0,
              UserManager *userManager = 0,
              PrivacyListManager *privacyListManager = 0,
              RosterManager *rosterManager = 0, VCardManager *vcardManager = 0,
              LastActivityManager *lastActivityManager = 0, EntityTimeManager *entityTimeManager = 0,
              PrivateStorageManager *privateStorageManager = 0,
              ServiceDiscoveryManager *serviceDiscoveryManager = 0,
              OfflineMessageManager *offlineMessageManager = 0,
              StreamNegotiationManager *streamNegotiationManager = 0,
              BlockingCommandManager *blockingCmdManager = 0, MucManager *mucManager = 0,
              ByteStreamsManager *byteStreamManager = 0, PubsubManager *pubsubManager = 0);

public slots:
    QByteArray parseIQ(QDomDocument document, QString from, QString host, QString streamId);

signals:
    void sigPresenceBroadCast(QString to, QDomDocument data);
    void sigRosterPush(QString jid, QDomDocument document);
    void sigSendReceiptRequest(QString to, QByteArray data);
    void sigResourceBinding(QString streamId, QString fullJid, QString id);
    void sigNonSaslAuthentification(QString streamId, QString fullJid, QString id);
    void sigStreamNegotiationError(QString streamId);
    void sigMucPresenceBroadCast(QString to, QDomDocument document);
    void sigGroupchatMessage(QString to, QDomDocument document);
    void sigOobRequest(QString to, QDomDocument document);
    void sigInbandByteStreamRequest(QString to, QDomDocument document);
    void sigApplicationReply(QString to, QDomDocument document);
    void sigApplicationRequest(QString to, QDomDocument document);
    void sigIqAvatarQuery(QString to, QDomDocument document);

private:
    QByteArray generateRoomAffiliationList(QString from, QString to, QString id, QList<QString> list,
                                           QString affiliation);
    QByteArray generateRoomRoleList(QString from, QString to, QString id, QList<QString> list,
                                           QString role);
    //QByteArray generateIQResult(QString from, QString to, QString id);
    QByteArray generateRosterGetResultReply(QString to, QString id, QList<Contact> rosterList);
    QByteArray generateIqSessionReply(QString id, QString from);
    QByteArray generateRegistrationFieldsReply(QString id);
    QByteArray generateAlreadyRegisterReply(QString username, QString password, QString id);
    QByteArray generatePongReply(QString from, QString to, QString id);
    QByteArray authentificationFields(QString id);
    QByteArray authenticate(QString streamId, QString id, QString username, QString password, QString resource, QString digest, QString host);
    QByteArray registerUserReply(QString username, QString password, QString jid, QString id, QDomElement firstChild, QString iqFrom);
    QDomDocument generateStorageAvatarDocument(QString from, QString to, QDomElement avatarElement);

    PresenceManager *m_presenceManager;
    UserManager *m_userManager;
    PrivacyListManager *m_privacyListManager;
    VCardManager *m_vCardManager;
    LastActivityManager *m_lastActivityManager;
    EntityTimeManager *m_entityTimeManager;
    RosterManager *m_rosterManager;
    PrivateStorageManager *m_privateStorageManager;
    ServiceDiscoveryManager *m_serviceDiscoveryManager;
    OfflineMessageManager *m_offlineMessageManager;
    StreamNegotiationManager *m_streamNegotiationManager;
    MucManager *m_mucManager;
    BlockingCommandManager *m_blockingCmdManager;
    ByteStreamsManager *m_byteStreamManager;
    PubsubManager *m_pubsubManager;
    QJsonObject *m_serverConfiguration;

};

#endif // IQMANAGER_H
