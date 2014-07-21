#include "PubsubManager.h"

PubsubManager::PubsubManager(RosterManager *rosterManager, StorageManager *storageManager)
{
    m_rosterManager = rosterManager;
    m_storageManager = storageManager;
}

QByteArray PubsubManager::pubsubManagerReply(QDomDocument document, QString iqFrom)
{
    QString from = document.documentElement().attribute("from", iqFrom);
    QString to = document.documentElement().attribute("to");
    QString id = document.documentElement().attribute("id", Utils::generateId());


    QString type = document.documentElement().attribute("type");

    if (type == "set")
    {
        QDomElement pubsubChild = document.documentElement().firstChildElement().firstChildElement();
        QString node = pubsubChild.attribute("node");
        QString jid = pubsubChild.attribute("jid");

        if (pubsubChild.tagName() == "subscribe")
        {
            if (!Utils::getBareJid(from).contains(Utils::getBareJid(jid)))
                return Error::generateError("", "iq", "modify", "bad-request", "invalid-jid", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            QString nodeModel = nodeAccessModel(node);
            if ((nodeModel == "presence"))
            {
                QString subscription = m_rosterManager->getContactSubscription(Utils::getBareJid(from), nodeOwner(node));
                if ((subscription != "to") && (subscription != "both"))
                    return Error::generateError("", "iq", "auth", "not-authorized", "presence-subscription-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);
            }

            if (nodeModel == "roster")
            {
                QStringList authRosterGroups = authorizedRosterGroups(node);
                QSet<QString> requesterRosterGroups = m_rosterManager->getContactGroups(nodeOwner(node), Utils::getBareJid(from));

                bool ok = false;
                foreach (QString group, requesterRosterGroups)
                {
                    if (authRosterGroups.contains(group))
                    {
                        ok = true;
                        break;
                    }
                }
                if (!ok)
                    return Error::generateError("", "iq", "auth", "not-authorized", "not-in-roster-group", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);
            }

            if (nodeModel == "whitelist")
            {
                QStringList whiteList = nodeWhiteList(node);
                if (!whiteList.contains(Utils::getBareJid(from)))
                    return Error::generateError("", "iq", "cancel", "not-allowed", "closed-node", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);
            }

            if (!nodeCustomerDatabase(node).contains(Utils::getBareJid(from)))
                return Error::generateError("", "iq", "auth", "payment-required", to, from, id, QDomElement());

            if (nodeUserSubscription(node, Utils::getBareJid(from)) == "pending")
                return Error::generateError("", "iq", "auth", "not-authorized", "pending-subscription", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            if (nodeUserAffiliation(node, Utils::getBareJid(from)) == "outcast")
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!allowSubscription(node))
                return Error::generateError("", "iq", "cancel", "feature-not-implemented", "unsupported", "http://jabber.org/protocol/pubsub#errors", "subscribe", to, from, id);

            if (!nodeExist(node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (nodeModel == "authorize")
                return pubsubSubscribeNotification(to, from, id, node, jid, "pending");

            if (configurationRequired(node))
                return pubsubUnconfiguredNotification(to, from, id, node, jid, true);

            if (subscribeToNode(node, jid))
            {
                return pubsubSubscribeNotification(to, from, id, node, jid, "subscribed") + "";
            }
        }
        else if (pubsubChild.tagName() == "unsubscribe")
        {

        }
    }
}

QByteArray PubsubManager::pubsubSubscribeNotification(QString from, QString to, QString id, QString node,
                                                      QString jid, QString subscription)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    iq.setAttribute("from", from);
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    QDomElement pubsub = document.createElement("pubsub");
    pubsub.setAttribute("xmlns", "http://jabber.org/protocol/pubsub");

    QDomElement subscriptionElement = document.createElement("subscription");
    subscriptionElement.setAttribute("node", node);
    subscriptionElement.setAttribute("jid", jid);
    subscriptionElement.setAttribute("subscription", subscription);

    pubsub.appendChild(subscriptionElement);
    iq.appendChild(pubsub);
    document.appendChild(iq);

    return document.toByteArray();
}

QByteArray PubsubManager::pubsubUnconfiguredNotification(QString from, QString to, QString id, QString node,
                                                         QString jid, bool required)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    iq.setAttribute("from", from);
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    QDomElement pubsub = document.createElement("pubsub");
    pubsub.setAttribute("xmlns", "http://jabber.org/protocol/pubsub");

    QDomElement subscription = document.createElement("subscription");
    subscription.setAttribute("node", node);
    subscription.setAttribute("jid", jid);
    subscription.setAttribute("subscription", "unconfigured");

    if (required)
    {
        QDomElement subscriptionOption = document.createElement("subscribe-options");
        subscriptionOption.appendChild(document.createElement("required"));
        subscription.appendChild(subscriptionOption);
    }

    pubsub.appendChild(subscription);
    iq.appendChild(pubsub);
    document.appendChild(iq);

    return document.toByteArray();
}

bool PubsubManager::subscribeToNode(QString node, QString jid)
{
    return m_storageManager->getStorage()->subscribeToNode(node, jid);
}

QString PubsubManager::nodeAccessModel(QString node)
{
    return m_storageManager->getStorage()->nodeAccessModel(node);
}

QString PubsubManager::nodeOwner(QString node)
{
    return m_storageManager->getStorage()->nodeOwner(node);
}

QStringList PubsubManager::authorizedRosterGroups(QString node)
{
    return m_storageManager->getStorage()->authorizedRosterGroups(node);
}

QStringList PubsubManager::nodeWhiteList(QString node)
{
    return m_storageManager->getStorage()->nodeWhiteList(node);
}

QStringList PubsubManager::nodeCustomerDatabase(QString node)
{
    return m_storageManager->getStorage()->nodeCustomerDatabase(node);
}

QString PubsubManager::nodeUserSubscription(QString node, QString jid)
{
    return m_storageManager->getStorage()->nodeUserSubscription(node, jid);
}

QString PubsubManager::nodeUserAffiliation(QString node, QString jid)
{
    return m_storageManager->getStorage()->nodeUserAffiliation(node, jid);
}

bool PubsubManager::allowSubscription(QString node)
{
    return m_storageManager->getStorage()->allowSubscription(node);
}

bool PubsubManager::nodeExist(QString node)
{
    return m_storageManager->getStorage()->nodeExist(node);
}

bool PubsubManager::configurationRequired(QString node)
{
    return m_storageManager->getStorage()->configurationRequired(node);
}
