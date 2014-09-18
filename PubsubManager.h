#ifndef PUBSUBMANAGER_H
#define PUBSUBMANAGER_H

#include <QObject>
#include <QDomDocument>
#include "RosterManager.h"
#include "Utils.h"
#include "Error.h"
#include "PubsubItem.h"
#include "NodeSubscriber.h"
#include "DataFormManager.h"
#include "InterestedPep.h"

class PubsubManager : public QObject
{
    Q_OBJECT
public:
    explicit PubsubManager(RosterManager *rosterManager = 0, StorageManager *storageManager = 0);
    QByteArray pubsubManagerReply(QDomDocument document, QString iqFrom);
    QList<PubsubItem> getNodeItems(QString pubsubService, QString node);

    void updateInterestedPepMap(QString jid1, InterestedPep pep);
    void deleteInterestedPepMap(QString jid);

signals:
    void sigPubsubNotification(QString to, QByteArray notification);

private:
    QByteArray pubsubSubscribeNotification(QString from, QString to, QString id, QString node, QString jid,
                                           QString subscription, QDomElement child) ;
    QByteArray pubsubUnconfiguredNotification(QString from, QString to, QString id, QString node, QString jid, bool required);
    QByteArray pubsubItemsPush(QString from, QString to, QString id, QString node, QList<PubsubItem> items);
    QByteArray pubsubItemPublishResult(QString from, QString to, QString id, QString node, QString itemId);
    QByteArray pubsubItemPublishNotification(QString from, QString to, QString id, QString node, PubsubItem item);
    QByteArray pubsubItemPublishNotification(QString from, QString to, QString id, QString node, QString itemId);
    QByteArray pubsubItemRetractNotification(QString from, QString to, QString id, QString node, QString itemId);
    QByteArray pubsubNodeConfigurationNotification(QString from, QString to, QString id, QString node, QDomElement xElement);
    QByteArray pubsubNodeDeleteNotification(QString from, QString to, QString id, QString node, QString redirectUri);
    QByteArray pubsubNodePurgeNotification(QString from, QString to, QString id, QString node);
    QByteArray pubsubNodeSubscriptionList(QString from, QString to, QString id, QString node,
                                          QList<NodeSubscriber> subscriptionList, bool owner);
    QByteArray pubsubAllNodeSubscriptionList(QString from, QString to, QString id,
                                          QMultiHash<QString, QList<NodeSubscriber> > nodeMapSubscriptionList);
    QByteArray pubsubNodeAffiliationList(QString from, QString to, QString id, QString node,
                                          QList<NodeSubscriber> affiliationList, bool owner);
    QByteArray pubsubAllNodeAffiliationList(QString from, QString to, QString id,
                                          QMultiHash<QString, QList<NodeSubscriber> > nodeMapAffiliationList);
    QByteArray pubsubChangeSubscriptionError(QString from, QString to, QString id, QString node,
                                             QList<QDomElement> subscriptionErrorList);
    QByteArray pubsubChangeSubscriptionNotification(QString from, QString to, QString node, QString jid,
                                                    QString subscription);

    QByteArray pubsubChangeAffiliationError(QString from, QString to, QString id, QString node,
                                             QList<QDomElement> affiliationErrorList);
    QByteArray pubsubChangeAffiliationNotification(QString from, QString to, QString node, QString jid,
                                                    QString affiliation);

    QStringList pubsubNodeList(QString pubsubService);
    bool subscribeToNode(QString pubsubService, QString node, NodeSubscriber subscriber);
    bool unsubscribeToNode(QString pubsubService, QString node, QString jid);
    QString nodeAccessModel(QString pubsubService, QString node);
    QString nodeOwner(QString pubsubService, QString node);
    QStringList authorizedRosterGroups(QString pubsubService, QString node);
    QStringList nodeWhiteList(QString pubsubService, QString node);
    QStringList nodeCustomerDatabase(QString pubsubService, QString node);
    QString nodeUserSubscription(QString pubsubService, QString node, QString jid);
    QString nodeUserAffiliation(QString pubsubService, QString node, QString jid);
    bool allowSubscription(QString pubsubService, QString node);
    bool nodeExist(QString pubsubService, QString node);
    bool configurationRequired(QString pubsubService, QString node);
    PubsubItem nodeLastPublishedItem(QString pubsubService, QString node);
    bool hasSubscription(QString pubsubService, QString node, QString jid);
    QMultiMap<QString, QVariant> nodeSubscriptionOptionForm(QString pubsubService, QString node, QString jid);
    bool processSubscriptionOptionForm(QString pubsubService, QString node, QString jid, QMultiMap<QString, QVariant> dataFormValues);
    QList<PubsubItem> getNodeItems(QString pubsubService, QString node, int max_items);
    PubsubItem getNodeItem(QString pubsubService, QString node, QString itemId);
    bool publishItem(QString pubsubService, QString node, PubsubItem item);
    bool notificationWithPayload(QString pubsubService, QString node);
    QStringList getSubscriberList(QString pubsubService, QString node);
    bool deleteItemToNode(QString pubsubService, QString node, QString itemId);
    bool createNode(QString pubsubService, QString node, QString owner,
                    QMultiMap<QString, QVariant> dataFormValue);
    QMultiMap<QString, QVariant> getNodeConfiguration(QString pubsubService, QString node);
    bool processNodeConfigurationForm(QString pubsubService, QString node, QMultiMap<QString, QVariant> dataFormValues);
    bool deleteNode(QString pubsubService, QString node);
    bool purgeNodeItems(QString pubsubService, QString node);
    bool notifyWhenItemRemove(QString pubsubService, QString node);
    bool nodePersistItems(QString pubsubService, QString node);
    QList<NodeSubscriber> nodeSubscriptionList(QString pubsubService, QString node);
    QList<NodeSubscriber> nodeAffiliationList(QString pubsubService, QString node);
    bool changeAffiliation(QString pubsubService, QString node, QString jid, QString affiliation);
    bool changeSubscription(QString pubsubService, QString node, QString jid, QString subscription);

    RosterManager *m_rosterManager;
    StorageManager *m_storageManager;
    QMultiHash<QString, InterestedPep> m_interestedPepMap;
};

#endif // PUBSUBMANAGER_H
