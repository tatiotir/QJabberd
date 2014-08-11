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

    QString pubsubService = to;

    QString type = document.documentElement().attribute("type");
    QDomElement pubsubChild = document.documentElement().firstChildElement().firstChildElement();
    if (type == "set")
    {
        QString node = pubsubChild.attribute("node");
        QString jid = pubsubChild.attribute("jid");

        if (pubsubChild.tagName() == "subscribe")
        {
            if (!Utils::getBareJid(from).contains(Utils::getBareJid(jid)))
                return Error::generateError("", "iq", "modify", "bad-request", "invalid-jid", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            QString nodeModel = nodeAccessModel(pubsubService, node);
            if ((nodeModel == "presence"))
            {
                QString subscription = m_rosterManager->getContactSubscription(Utils::getBareJid(from), nodeOwner(pubsubService, node));
                if ((subscription != "to") && (subscription != "both"))
                    return Error::generateError("", "iq", "auth", "not-authorized", "presence-subscription-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);
            }

            if (nodeModel == "roster")
            {
                QStringList authRosterGroups = authorizedRosterGroups(pubsubService, node);
                QSet<QString> requesterRosterGroups = m_rosterManager->getContactGroups(nodeOwner(pubsubService, node), Utils::getBareJid(from));

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
                QStringList whiteList = nodeWhiteList(pubsubService, node);
                if (!whiteList.contains(Utils::getBareJid(from)))
                    return Error::generateError("", "iq", "cancel", "not-allowed", "closed-node", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);
            }

//            if (!nodeCustomerDatabase(pubsubService, node).contains(Utils::getBareJid(from)))
//                return Error::generateError("", "iq", "auth", "payment-required", to, from, id, QDomElement());

            if (nodeUserSubscription(pubsubService, node, Utils::getBareJid(from)) == "pending")
                return Error::generateError("", "iq", "auth", "not-authorized", "pending-subscription", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            QString affiliation = nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from));
            if ((affiliation == "outcast") || (affiliation == "publish-only"))
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!allowSubscription(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "feature-not-implemented", "unsupported", "http://jabber.org/protocol/pubsub#errors", "subscribe", to, from, id);

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (nodeModel == "authorize")
                return pubsubSubscribeNotification(to, from, id, node, jid, "pending", QDomElement());

            if (configurationRequired(pubsubService, node))
                return pubsubUnconfiguredNotification(to, from, id, node, jid, true);

            if (subscribeToNode(pubsubService, node, NodeSubscriber(jid, "member", "subscribed")))
            {
                if (pubsubChild.firstChildElement().tagName() == "option")
                {
                    QMultiMap<QString, QVariant> dataFormValues = DataFormManager::parseDataForm(pubsubChild.firstChildElement().firstChildElement());
                    if (processSubscriptionOptionForm(pubsubService, node, jid, dataFormValues))
                    {
                        pubsubChild.firstChildElement().firstChildElement().setAttribute("type", "result");
                        pubsubSubscribeNotification(to, from, id, node, jid, "subscribed", pubsubChild.firstChildElement().firstChildElement())
                                + pubsubItemPublishNotification(to, from, id, node, nodeLastPublishedItem(pubsubService, node));
                    }
                }
                else
                {
                    QByteArray reply = pubsubSubscribeNotification(to, from, id, node, jid, "subscribed", QDomElement());
                    PubsubItem item = nodeLastPublishedItem(pubsubService, node);
                    if (!item.id().isEmpty())
                        reply += pubsubItemPublishNotification(to, from, id, node, item);
                    return reply;
                }
            }
        }
        else if (pubsubChild.tagName() == "unsubscribe")
        {
            if (!hasSubscription(pubsubService, node, jid))
                return Error::generateError("", "iq", "cancel", "unexpected-request", "not-subscribed", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (unsubscribeToNode(pubsubService, node, jid))
                return Utils::generateIQResult(to, from, id);
        }
        else if (pubsubChild.tagName() == "options")
        {
            QString node = pubsubChild.attribute("node");
            QString jid = pubsubChild.attribute("jid");

            QMultiMap<QString, QVariant> dataFormValues = DataFormManager::parseDataForm(pubsubChild.firstChildElement());
            if (processSubscriptionOptionForm(pubsubService, node, jid, dataFormValues))
                return Utils::generateIQResult(to, from, id);
        }
        else if (pubsubChild.tagName() == "publish")
        {
            QString affiliation = nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from));
            if ((affiliation == "outcast") || (affiliation == "member") || (affiliation == "none"))
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (pubsubChild.childNodes().count() > 1)
                return Error::generateError("", "iq", "modify", "bad-request", "invalid-payload", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            if (pubsubChild.childNodes().count() == 0)
                return Error::generateError("", "iq", "modify", "bad-request", "item-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            if (pubsubChild.firstChildElement().childNodes().count() == 0)
                return Error::generateError("", "iq", "modify", "bad-request", "payload-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            QString itemId = pubsubChild.firstChildElement().attribute("id", Utils::generateId());
            QByteArray itemData;

            QTextStream stream(&itemData);
            pubsubChild.firstChildElement().save(stream, 0);

            if (publishItem(pubsubService, node, PubsubItem(itemId, itemData)))
            {
                QStringList subscribers = getSubscriberList(pubsubService, node);
                QByteArray notification;
                if (notificationWithPayload(pubsubService, node))
                {
                    notification = pubsubItemPublishNotification(to, from, id, node, PubsubItem(node, itemData));
                }
                else
                {
                    notification = pubsubItemPublishNotification(to, from, id, node, itemId);
                }

                foreach (QString subscriber, subscribers)
                {
                    emit sigPubsubNotification(subscriber, notification);
                }

                return pubsubItemPublishResult(to, from, id, node, itemId);
            }
        }
        else if (pubsubChild.tagName() == "retract")
        {
            QString affiliation = nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from));
            if ((affiliation == "outcast") || (affiliation == "member") || (affiliation == "none"))
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (node.isEmpty())
                return Error::generateError("", "iq", "modify", "bad-request", "nodeid-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            QString itemId = pubsubChild.firstChildElement().attribute("id");

            if (itemId.isEmpty())
                return Error::generateError("", "iq", "modify", "bad-request", "item-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            QStringList subscribers = getSubscriberList(pubsubService, node);
            if (deleteItemToNode(pubsubService, node, itemId))
            {
                if ((pubsubChild.attribute("notify") == "true") || (pubsubChild.attribute("notify") == "1"))
                {
                    QByteArray notification = pubsubItemRetractNotification(to, from, id, node, itemId);
                    foreach (QString subscriber, subscribers)
                    {
                        emit sigPubsubNotification(subscriber, notification);
                    }
                }
                return Utils::generateIQResult(to, from, id);
            }
        }
        else if (pubsubChild.tagName() == "create")
        {
            if (nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "conflict", to, from, id, QDomElement());

            QMultiMap<QString, QVariant> dataFormValues = DataFormManager::parseDataForm(pubsubChild.firstChildElement().firstChildElement());

            if (!dataFormValues.isEmpty())
            {
                QString accessModel = dataFormValues.value("pubsub#access_model").toString();
                if ((accessModel != "authorize") || (accessModel != "open") || (accessModel != "presence")
                        || (accessModel != "roster") || (accessModel != "whitelist"))
                    return Error::generateError("", "iq", "modify", "not-acceptable", "unsupported-access-model", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);
            }

            if (createNode(pubsubService, node, Utils::getBareJid(from), dataFormValues))
                return Utils::generateIQResult(to, from, id);
        }
        else if (pubsubChild.tagName() == "configure")
        {
            QString affiliation = nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from));
            if ((affiliation != "owner"))
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            QMultiMap<QString, QVariant> dataFormValue = DataFormManager::parseDataForm(pubsubChild.firstChildElement());
            if (processNodeConfigurationForm(pubsubService, node, dataFormValue))
            {
                if (dataFormValue.value("pubsub#notify_config").toBool())
                {
                    QStringList subscribers = getSubscriberList(pubsubService, node);

                    pubsubChild.firstChildElement().setAttribute("type", "result");
                    QByteArray notification;
                    if (dataFormValue.value("pubsub#deliver_payloads").toBool())
                        notification = pubsubNodeConfigurationNotification(to, from, id, node, pubsubChild.firstChildElement());
                    else
                        notification = pubsubNodeConfigurationNotification(to, from, id, node, QDomElement());

                    foreach (QString subscriber, subscribers)
                    {
                        emit sigPubsubNotification(subscriber, notification);
                    }
                }
                return Utils::generateIQResult(to, from, id);
            }
        }
        else if (pubsubChild.tagName() == "delete")
        {
            QString affiliation = nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from));
            if ((affiliation != "owner"))
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (deleteNode(pubsubService, node))
            {
                QByteArray notification = pubsubNodeDeleteNotification(to, from, id, node, pubsubChild.firstChildElement().attribute("uri"));
                QStringList subscribers = getSubscriberList(pubsubService, node);
                foreach (QString subscriber, subscribers)
                {
                    emit sigPubsubNotification(subscriber, notification);
                }

                return Utils::generateIQResult(to, from, id);
            }
        }
        else if (pubsubChild.tagName() == "purge")
        {
            QString affiliation = nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from));
            if ((affiliation != "owner"))
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (!nodePersistItems(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "feature-not-implemented", "unsupported", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            if (purgeNodeItems(pubsubService, node))
            {
                if (notifyWhenItemRemove(pubsubService, node))
                {
                    QByteArray notification = pubsubNodePurgeNotification(to, from, id, node);
                    QStringList subscribers = getSubscriberList(pubsubService, node);
                    foreach (QString subscriber, subscribers)
                    {
                        emit sigPubsubNotification(subscriber, notification);
                    }

                    return Utils::generateIQResult(to, from, id);
                }
            }
        }
    }
    else if (type == "get")
    {
        QString node = pubsubChild.attribute("node");
        if (pubsubChild.tagName() == "options")
        {
            QString jid = pubsubChild.attribute("jid");

            QString affiliation = nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from));
            if ((affiliation == "outcast") || (affiliation == "member") || (affiliation == "none"))
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!hasSubscription(pubsubService, node, jid))
                return Error::generateError("", "iq", "cancel", "unexpected-request", "not-subscribed", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            if (jid.isEmpty())
                return Error::generateError("", "iq", "cancel", "bad-request", "jid-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            return DataFormManager::getNodeSubscriptionOptionForm("result", to, from, id, node, jid, nodeSubscriptionOptionForm(pubsubService, node, jid)).toByteArray();
        }
        else if (pubsubChild.tagName() == "default")
        {
            return Error::generateError("", "iq", "cancel", "feature-not-implemented", "unsupported", "http://jabber.org/protocol/pubsub#errors", "retrieve-default-sub", to, from, id);
        }
        else if (pubsubChild.tagName() == "items")
        {
            QString node = pubsubChild.attribute("node");

            if (!hasSubscription(pubsubService, node, from))
                return Error::generateError("", "iq", "auth", "not-authorized", "not-subscribed", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            QString nodeModel = nodeAccessModel(pubsubService, node);
            if ((nodeModel == "presence"))
            {
                QString subscription = m_rosterManager->getContactSubscription(Utils::getBareJid(from), nodeOwner(pubsubService, node));
                if ((subscription != "to") && (subscription != "both"))
                    return Error::generateError("", "iq", "auth", "not-authorized", "presence-subscription-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);
            }

            if (nodeModel == "roster")
            {
                QStringList authRosterGroups = authorizedRosterGroups(pubsubService, node);
                QSet<QString> requesterRosterGroups = m_rosterManager->getContactGroups(nodeOwner(pubsubService, node), Utils::getBareJid(from));

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
                QStringList whiteList = nodeWhiteList(pubsubService, node);
                if (!whiteList.contains(Utils::getBareJid(from)))
                    return Error::generateError("", "iq", "cancel", "not-allowed", "closed-node", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);
            }

//            if (!nodeCustomerDatabase(pubsubService, node).contains(Utils::getBareJid(from)))
//                return Error::generateError("", "iq", "auth", "payment-required", to, from, id, QDomElement());

            QString affiliation = nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from));
            if ((affiliation == "outcast") || (affiliation == "member") || (affiliation == "none"))
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (!pubsubChild.hasChildNodes())
            {
                if (pubsubChild.hasAttribute("max_items"))
                {
                    return pubsubItemsPush(to, from, id, node, getNodeItems(pubsubService, node, pubsubChild.attribute("max_items").toInt()));
                }
                return pubsubItemsPush(to, from, id, node, getNodeItems(pubsubService, node));
            }
            else
            {
                QList<PubsubItem> pubsubItems;
                QDomNodeList itemsChilds = pubsubChild.childNodes();
                for (int i = 0; i < itemsChilds.count(); ++i)
                {
                    pubsubItems << getNodeItem(pubsubService, node, itemsChilds.at(i).toElement().attribute("id"));
                }
                return pubsubItemsPush(to, from, id, node, pubsubItems);
            }
        }
        else if (pubsubChild.tagName() == "configure")
        {
            if (nodeUserAffiliation(pubsubService, node, Utils::getBareJid(from)) == "outcast")
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());

            if (!nodeExist(pubsubService, node))
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, QDomElement());

            if (node.isEmpty())
                return Error::generateError("", "iq", "modify", "bad-request", "nodeid-required", "http://jabber.org/protocol/pubsub#errors", "", to, from, id);

            return DataFormManager::getNodeConfigurationForm("result", to, from, id, node, getNodeConfiguration(pubsubService, node), m_rosterManager->getGroups(Utils::getBareJid(from))).toByteArray();
        }
    }
    return QByteArray();
}

QByteArray PubsubManager::pubsubSubscribeNotification(QString from, QString to, QString id, QString node, QString jid,
                                                      QString subscription, QDomElement child)
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

    if (!child.isNull())
        subscriptionElement.appendChild(document.importNode(child, true));

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

QByteArray PubsubManager::pubsubItemsPush(QString from, QString to, QString id, QString node, QList<PubsubItem> items)
{
    QDomDocument document;
    QDomElement iq = document.createElement("message");

    iq.setAttribute("from", from);
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    QDomElement pubsubElement = document.createElement("pubsub");
    pubsubElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub");

    QDomElement itemsElement = document.createElement("items");
    itemsElement.setAttribute("node", node);

    foreach (PubsubItem item, items)
    {
        QDomElement itemElement = document.createElement("item");
        itemElement.setAttribute("id", item.id());

        QDomDocument doc;
        doc.setContent(item.data());

        itemElement.appendChild(document.importNode(document.documentElement(), true));
        itemsElement.appendChild(itemElement);
    }

    pubsubElement.appendChild(itemsElement);
    iq.appendChild(pubsubElement);
    document.appendChild(iq);

    return document.toByteArray();
}

QByteArray PubsubManager::pubsubItemPublishNotification(QString from, QString to, QString id, QString node, PubsubItem item)
{
    QDomDocument document;
    QDomElement message = document.createElement("iq");

    message.setAttribute("from", from);
    message.setAttribute("to", to);
    message.setAttribute("id", id);
    message.setAttribute("type", "result");

    QDomElement eventElement = document.createElement("pubsub");
    eventElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub#event");

    QDomElement itemsElement = document.createElement("items");
    itemsElement.setAttribute("node", node);

    QDomElement itemElement = document.createElement("item");
    itemElement.setAttribute("id", item.id());

    QDomDocument doc;
    doc.setContent(item.data());

    itemElement.appendChild(document.importNode(document.documentElement(), true));
    itemsElement.appendChild(itemElement);

    eventElement.appendChild(itemsElement);
    message.appendChild(eventElement);
    document.appendChild(message);

    return document.toByteArray();
}

QByteArray PubsubManager::pubsubItemPublishNotification(QString from, QString to, QString id, QString node,
                                                        QString itemId)
{
    QDomDocument document;
    QDomElement message = document.createElement("message");

    message.setAttribute("from", from);
    message.setAttribute("to", to);
    message.setAttribute("id", id);

    QDomElement eventElement = document.createElement("pubsub");
    eventElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub#event");

    QDomElement itemsElement = document.createElement("items");
    itemsElement.setAttribute("node", node);

    QDomElement itemElement = document.createElement("item");
    itemElement.setAttribute("id", itemId);

    itemsElement.appendChild(itemElement);
    eventElement.appendChild(itemsElement);
    message.appendChild(eventElement);
    document.appendChild(message);

    return document.toByteArray();
}

QByteArray PubsubManager::pubsubItemRetractNotification(QString from, QString to, QString id, QString node, QString itemId)
{
    QDomDocument document;
    QDomElement message = document.createElement("message");

    message.setAttribute("from", from);
    message.setAttribute("to", to);
    message.setAttribute("id", id);

    QDomElement eventElement = document.createElement("pubsub");
    eventElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub#event");

    QDomElement itemsElement = document.createElement("items");
    itemsElement.setAttribute("node", node);

    QDomElement retractElement = document.createElement("retract");
    retractElement.setAttribute("id", itemId);

    itemsElement.appendChild(retractElement);
    eventElement.appendChild(itemsElement);
    message.appendChild(eventElement);
    document.appendChild(message);

    return document.toByteArray();
}

QByteArray PubsubManager::pubsubNodeConfigurationNotification(QString from, QString to, QString id, QString node,
                                                              QDomElement xElement)
{
    QDomDocument document;
    QDomElement message = document.createElement("message");

    message.setAttribute("from", from);
    message.setAttribute("to", to);
    message.setAttribute("id", id);

    QDomElement eventElement = document.createElement("pubsub");
    eventElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub#event");

    QDomElement configurationElement = document.createElement("configuration");
    configurationElement.setAttribute("node", node);

    if (!xElement.isNull())
        configurationElement.appendChild(document.importNode(xElement, true));

    eventElement.appendChild(configurationElement);
    message.appendChild(eventElement);
    document.appendChild(message);

    return document.toByteArray();
}

QByteArray PubsubManager::pubsubItemPublishResult(QString from, QString to, QString id, QString node, QString itemId)
{
    QDomDocument document;
    QDomElement iq = document.createElement("message");

    iq.setAttribute("from", from);
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    QDomElement pubsubElement = document.createElement("pubsub");
    pubsubElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub");

    QDomElement publishElement = document.createElement("publish");
    publishElement.setAttribute("node", node);

    QDomElement itemElement = document.createElement("item");
    itemElement.setAttribute("id", itemId);

    publishElement.appendChild(itemElement);
    pubsubElement.appendChild(publishElement);
    iq.appendChild(pubsubElement);
    document.appendChild(iq);

    return document.toByteArray();
}

QByteArray PubsubManager::pubsubNodeDeleteNotification(QString from, QString to, QString id, QString node, QString redirectUri)
{
    QDomDocument document;
    QDomElement message = document.createElement("message");

    message.setAttribute("from", from);
    message.setAttribute("to", to);
    message.setAttribute("id", id);

    QDomElement eventElement = document.createElement("pubsub");
    eventElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub#event");

    QDomElement deleteElement = document.createElement("delete");
    deleteElement.setAttribute("node", node);

    QDomElement redirectElement = document.createElement("redirect");
    redirectElement.setAttribute("uri", redirectUri);

    deleteElement.appendChild(redirectElement);
    eventElement.appendChild(deleteElement);
    message.appendChild(eventElement);
    document.appendChild(message);

    return document.toByteArray();
}

QByteArray PubsubManager::pubsubNodePurgeNotification(QString from, QString to, QString id, QString node)
{
    QDomDocument document;
    QDomElement message = document.createElement("message");

    message.setAttribute("from", from);
    message.setAttribute("to", to);
    message.setAttribute("id", id);

    QDomElement eventElement = document.createElement("pubsub");
    eventElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub#event");

    QDomElement purgeElement = document.createElement("purge");
    purgeElement.setAttribute("node", node);

    eventElement.appendChild(purgeElement);
    message.appendChild(eventElement);
    document.appendChild(message);

    return document.toByteArray();
}

bool PubsubManager::subscribeToNode(QString pubsubService, QString node, NodeSubscriber subscriber)
{
    return m_storageManager->getStorage()->subscribeToNode(pubsubService, node, subscriber);
}

bool PubsubManager::unsubscribeToNode(QString pubsubService, QString node, QString jid)
{
    return m_storageManager->getStorage()->unsubscribeToNode(pubsubService, node, jid);
}

QString PubsubManager::nodeAccessModel(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->nodeAccessModel(pubsubService, node);
}

QString PubsubManager::nodeOwner(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->nodeOwner(pubsubService, node);
}

QStringList PubsubManager::authorizedRosterGroups(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->authorizedRosterGroups(pubsubService, node);
}

QStringList PubsubManager::nodeWhiteList(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->nodeWhiteList(pubsubService, node);
}

QStringList PubsubManager::nodeCustomerDatabase(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->nodeCustomerDatabase(pubsubService, node);
}

QString PubsubManager::nodeUserSubscription(QString pubsubService, QString node, QString jid)
{
    return m_storageManager->getStorage()->nodeUserSubscription(pubsubService, node, jid);
}

QString PubsubManager::nodeUserAffiliation(QString pubsubService, QString node, QString jid)
{
    return m_storageManager->getStorage()->nodeUserAffiliation(pubsubService, node, jid);
}

bool PubsubManager::allowSubscription(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->allowSubscription(pubsubService, node);
}

bool PubsubManager::nodeExist(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->nodeExist(pubsubService, node);
}

bool PubsubManager::configurationRequired(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->configurationRequired(pubsubService, node);
}

PubsubItem PubsubManager::nodeLastPublishedItem(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->nodeLastPublishedItem(pubsubService, node);
}

bool PubsubManager::hasSubscription(QString pubsubService, QString node, QString jid)
{
    return m_storageManager->getStorage()->hasSubscription(pubsubService, node, jid);
}

QMultiMap<QString, QVariant> PubsubManager::nodeSubscriptionOptionForm(QString pubsubService, QString node, QString jid)
{
    return m_storageManager->getStorage()->nodeSubscriptionOptionForm(pubsubService, node, jid);
}

bool PubsubManager::processSubscriptionOptionForm(QString pubsubService, QString node, QString jid, QMultiMap<QString, QVariant> dataFormValues)
{
    return m_storageManager->getStorage()->processSubscriptionOptionForm(pubsubService, node, jid, dataFormValues);
}

QList<PubsubItem> PubsubManager::getNodeItems(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->getNodeItems(pubsubService, node);
}

QList<PubsubItem> PubsubManager::getNodeItems(QString pubsubService, QString node, int max_items)
{
    return m_storageManager->getStorage()->getNodeItems(pubsubService, node, max_items);
}

PubsubItem PubsubManager::getNodeItem(QString pubsubService, QString node, QString itemId)
{
    return m_storageManager->getStorage()->getNodeItem(pubsubService, node, itemId);
}

bool PubsubManager::publishItem(QString pubsubService, QString node, PubsubItem item)
{
    return m_storageManager->getStorage()->publishItem(pubsubService, node, item);
}

bool PubsubManager::notificationWithPayload(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->notificationWithPayload(pubsubService, node);
}

QStringList PubsubManager::getSubscriberList(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->getSubscriberList(pubsubService, node);
}

bool PubsubManager::deleteItemToNode(QString pubsubService, QString node, QString itemId)
{
    return m_storageManager->getStorage()->deleteItemToNode(pubsubService, node, itemId);
}

bool PubsubManager::createNode(QString pubsubService, QString node, QString owner, QMultiMap<QString, QVariant> dataFormValue)
{
    return m_storageManager->getStorage()->createNode(pubsubService, node, owner, dataFormValue);
}

QMultiMap<QString, QVariant> PubsubManager::getNodeConfiguration(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->getNodeConfiguration(pubsubService, node);
}

bool PubsubManager::processNodeConfigurationForm(QString pubsubService, QString node, QMultiMap<QString, QVariant> dataFormValues)
{
    return m_storageManager->getStorage()->processNodeConfigurationForm(pubsubService, node, dataFormValues);
}

bool PubsubManager::deleteNode(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->deleteNode(pubsubService, node);
}

bool PubsubManager::purgeNodeItems(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->purgeNodeItems(pubsubService, node);
}

bool PubsubManager::notifyWhenItemRemove(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->notifyWhenItemRemove(pubsubService, node);
}

bool PubsubManager::nodePersistItems(QString pubsubService, QString node)
{
    return m_storageManager->getStorage()->nodePersistItems(pubsubService, node);
}
