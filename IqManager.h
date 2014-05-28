#ifndef IQMANAGER_H
#define IQMANAGER_H

#include <QDebug>
#include <QString>
#include <QDomElement>
#include <QDomNodeList>
#include "UserManager.h"
#include "DataForm.h"
#include "Error.h"
#include "ServiceDiscoveryManager.h"
#include "PrivacyListManager.h"
#include "VcardManager.h"
#include "LastActivityManager.h"
#include "EntityTimeManager.h"
#include "RosterManager.h"
#include "PrivateStorageManager.h"
#include "OfflineMessageManager.h"
#include "StreamNegotiationManager.h"
#include "OobDataManager.h"

class IQManager : public QObject
{
    Q_OBJECT
public:
    IQManager(QMap<QString, QVariant> *serverConfigMap = 0, UserManager *userManager = 0, PrivacyListManager *privacyListManager = 0,
              RosterManager *rosterManager = 0, VCardManager *vcardManager = 0,
              LastActivityManager *lastActivityManager = 0, EntityTimeManager *entityTimeManager = 0,
              PrivateStorageManager *privateStorageManager = 0,
              ServiceDiscoveryManager *serviceDiscoveryManager = 0,
              OfflineMessageManager *offlineMessageManager = 0,
              StreamNegotiationManager *streamNegotiationManager = 0, OobDataManager *oobDataManager = 0);

public slots:
    QByteArray parseIQ(QByteArray iqXML, QString from, QString host, QString streamId);

signals:
    void sigPresenceBroadCast(QString to, QByteArray data);
    void sigRosterPush(QString jid, QByteArray data);
    void sigLastActivityQuery(QString from, QString to, QString id, QString lastStatus);
    void sigServerLastActivityQuery(QString from, QString to, QString id);
    void sigSendReceiptRequest(QString to, QByteArray data);
    void sigResourceBinding(QString streamId, QString fullJid, QString id);
    void sigNonSaslAuthentification(QString streamId, QString fullJid, QString id);
    void sigStreamNegotiationError(QString streamId);

private:
    QByteArray generateIQResult(QString to, QString id);
    QByteArray generateRosterGetResultReply(QString to, QString id, QList<Contact> rosterList);
    QByteArray generateIqSessionReply(QString id, QString from);
    QByteArray generateRegistrationFieldsReply(QString id);
    QByteArray generateAlreadyRegisterReply(QString username, QString password, QString id);
    QByteArray generatePongReply(QString from, QString to, QString id);
    QByteArray authentificationFields(QString id);
    QByteArray authenticate(QString streamId, QString id, QString username, QString password, QString resource, QString digest, QString host);

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
    OobDataManager *m_oobDataManager;
    QMap<QString, QVariant> *m_serverConfigMap;
};

#endif // IQMANAGER_H
