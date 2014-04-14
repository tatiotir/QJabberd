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

class IQManager : public QObject
{
    Q_OBJECT
public:
    IQManager(QMap<QString, QVariant> *serverConfigMap = 0, UserManager *userManager = 0, PrivacyListManager *privacyListManager = 0,
              RosterManager *rosterManager = 0, VCardManager *vcardManager = 0,
              LastActivityManager *lastActivityManager = 0, EntityTimeManager *entityTimeManager = 0,
              PrivateStorageManager *privateStorageManager = 0,
              ServiceDiscoveryManager *serviceDiscoveryManager = 0,
              OfflineMessageManager *offlineMessageManager = 0);

public slots:
    QByteArray parseIQ(QByteArray iqXML, QString from, QString host);
    QByteArray generateIQResult(QString to, QString id);
    QByteArray generateRosterGetResultReply(QString to, QString id, QList<Contact> rosterList);
    QByteArray generateIqSessionReply(QString id);
    QByteArray generateRegistrationFieldsReply(QString id);
    QByteArray generateAlreadyRegisterReply(QString username, QString password, QString id);
    QByteArray generatePongReply(QString from, QString to, QString id);

signals:
    void sigPresenceBroadCast(QString to, QByteArray data);
    void sigRosterPush(QString jid, QByteArray data);
    void sigLastActivityQuery(QString from, QString to, QString id, QString lastStatus);
    void sigServerLastActivityQuery(QString from, QString to, QString id);
    void sigSendReceiptRequest(QString to, QByteArray data);

private:
    UserManager *m_userManager;
    PrivacyListManager *m_privacyListManager;
    VCardManager *m_vCardManager;
    LastActivityManager *m_lastActivityManager;
    EntityTimeManager *m_entityTimeManager;
    RosterManager *m_rosterManager;
    PrivateStorageManager *m_privateStorageManager;
    ServiceDiscoveryManager *m_serviceDiscoveryManager;
    OfflineMessageManager *m_offlineMessageManager;
    QMap<QString, QVariant> *m_serverConfigMap;
};

#endif // IQMANAGER_H
