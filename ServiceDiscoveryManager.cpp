#include "ServiceDiscoveryManager.h"

ServiceDiscoveryManager::ServiceDiscoveryManager(QObject *parent, QJsonObject *serverConfiguration,
                                                 UserManager *userManager, MucManager *mucManager,
                                                 PubsubManager *pubsubManager) :
    QObject(parent)
{
    m_serverConfiguration = serverConfiguration;
    m_userManager = userManager;
    m_mucManager = mucManager;
    m_pubsubManager = pubsubManager;
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerReply(QDomDocument document, QString iqFrom)
{
    QDomElement iq = document.documentElement();
    QString to = iq.attribute("to");
    QString type = iq.attribute("type");
    QString from = iq.attribute("from", iqFrom);
    QString xmlns = iq.firstChildElement().attribute("xmlns");
    QString id = iq.attribute("id", Utils::generateId());

    if (xmlns == "http://jabber.org/protocol/disco#items")
    {
        QString node = iq.firstChildElement().attribute("node");
        if (type == "result")
        {
            emit sigClientServiceDiscoveryResponse(to, document.toByteArray());
            return QByteArray();
        }

        if (!node.isEmpty())
        {
            return serviceDiscoveryManagerItemsQueryResult(node, Utils::getBareJid(iqFrom), document);
        }
        else
        {
            return serviceDiscoveryManagerItemsQueryResult(from, to, id);
        }
    }
    else if (xmlns == "http://jabber.org/protocol/disco#info")
    {
        if (type == "result")
        {
            InterestedPep pep;
            pep.setJid(from);

            QDomNodeList featureList = document.documentElement().elementsByTagName("feature");
            for (int i = 0; i < featureList.count(); ++i)
            {
                QDomElement featureElement = featureList.item(i).toElement();
                QString var = featureElement.attribute("var");

                if (var.right(11) == "mood+notify")
                {
                    pep.setMood(true);
                }
                else if (var.right(11) == "tune+notify")
                {
                    pep.setTune(true);
                }
                else if (var.right(11) == "physloc+notify")
                {
                    pep.setPhysloc(true);
                }
                else if (var.right(11) == "geoloc+notify")
                {
                    pep.setGeoloc(true);
                }
            }
            m_pubsubManager->updateInterestedPepMap(Utils::getBareJid(to), pep);
            emit sigClientServiceDiscoveryResponse(to, document.toByteArray());
            return QByteArray();
        }

        QString node = iq.firstChildElement().attribute("node");

        if (node == "x-roomuser-item")
        {
            return Error::generateError("", "iq", "cancel", "feature-not-implemented", to, from, id, QDomElement());
        }

        if (!node.isEmpty())
        {
            return serviceDiscoveryManagerInfoQueryResult(iqFrom, node, document);
        }
        else
        {
            return serviceDiscoveryManagerInfoQueryResult(from, to, id, document);
        }
    }
    return QByteArray();
}


QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerInfoQueryResult(QString from, QString to, QString id,
                                                                           QDomDocument document)
{
    if (m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(to))
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");
        iq.setAttribute("from", to);
        iq.setAttribute("to", from);
        iq.setAttribute("id", id);
        iq.setAttribute("type", "result");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");

        QDomElement identity1 = document.createElement("identity");
        identity1.setAttribute("category", "server");
        identity1.setAttribute("type", "im");
        identity1.setAttribute("name", "QJabberd");

        QDomElement identity2 = document.createElement("identity");
        identity2.setAttribute("category", "pubsub");
        identity2.setAttribute("type", "pep");
        identity2.setAttribute("name", "QJabberd");

        if (m_serverConfiguration->value("modules").toObject().value("pubsub").toBool())
        {
            query.appendChild(identity2);
        }

        query.appendChild(identity1);

        if (m_serverConfiguration->value("modules").toObject().value("disco").toBool())
        {
            QDomElement feature1 = document.createElement("feature");
            feature1.setAttribute("var", "http://jabber.org/protocol/disco#info");

            QDomElement feature2 = document.createElement("feature");
            feature2.setAttribute("var", "http://jabber.org/protocol/disco#items");

            query.appendChild(feature1);
            query.appendChild(feature2);
        }

        if (m_serverConfiguration->value("modules").toObject().value("register").toBool())
        {
            QDomElement feature3 = document.createElement("feature");
            feature3.setAttribute("var", "jabber:iq:register");

            query.appendChild(feature3);
        }

        if (m_serverConfiguration->value("modules").toObject().value("ping").toBool())
        {
            QDomElement feature4 = document.createElement("feature");
            feature4.setAttribute("var", "urn:xmpp:ping");

            query.appendChild(feature4);
        }

        if (m_serverConfiguration->value("modules").toObject().value("vcard-temp").toBool())
        {
            QDomElement feature5 = document.createElement("feature");
            feature5.setAttribute("var", "vcard-temp");

            query.appendChild(feature5);
        }

        if (m_serverConfiguration->value("modules").toObject().value("private").toBool())
        {
            QDomElement feature6 = document.createElement("feature");
            feature6.setAttribute("var", "jabber:iq:private");

            query.appendChild(feature6);
        }

        if (m_serverConfiguration->value("modules").toObject().value("lastActivity").toBool())
        {
            QDomElement feature7 = document.createElement("feature");
            feature7.setAttribute("var", "jabber:iq:last");

            query.appendChild(feature7);
        }

        if (m_serverConfiguration->value("modules").toObject().value("roster").toBool())
        {
            QDomElement feature8 = document.createElement("feature");
            feature8.setAttribute("var", "jabber:iq:roster");

            query.appendChild(feature8);
        }

        if (m_serverConfiguration->value("modules").toObject().value("time").toBool())
        {
            QDomElement feature9 = document.createElement("feature");
            feature9.setAttribute("var", "urn:xmpp:time");

            query.appendChild(feature9);
        }

        if (m_serverConfiguration->value("modules").toObject().value("offline").toBool())
        {
            QDomElement feature10 = document.createElement("feature");
            feature10.setAttribute("var", "http://jabber.org/protocol/offline");

            query.appendChild(feature10);
        }

        if (m_serverConfiguration->value("modules").toObject().value("msgoffline").toBool())
        {
            QDomElement feature11 = document.createElement("feature");
            feature11.setAttribute("var", "msgoffline");

            query.appendChild(feature11);
        }

        if (m_serverConfiguration->value("modules").toObject().value("nonsaslauth").toBool())
        {
            QDomElement feature12 = document.createElement("feature");
            feature12.setAttribute("var", "jabber:iq:auth");

            query.appendChild(feature12);
        }

        if (m_serverConfiguration->value("modules").toObject().value("si").toBool())
        {
            QDomElement feature13 = document.createElement("feature");
            feature13.setAttribute("var", "http://jabber.org/protocol/si");

            query.appendChild(feature13);
        }

        if (m_serverConfiguration->value("modules").toObject().value("file-transfert").toBool())
        {
            QDomElement feature14 = document.createElement("feature");
            feature14.setAttribute("var", "http://jabber.org/protocol/file-transfert");

            query.appendChild(feature14);
        }

        if (m_serverConfiguration->value("modules").toObject().value("ibb").toBool())
        {
            QDomElement feature13 = document.createElement("feature");
            feature13.setAttribute("var", "http://jabber.org/protocol/ibb");

            query.appendChild(feature13);
        }

        if (m_serverConfiguration->value("modules").toObject().value("oob").toBool())
        {
            QDomElement feature14 = document.createElement("feature");
            feature14.setAttribute("var", "http://jabber.org/protocol/oob");

            query.appendChild(feature14);
        }

        if (m_serverConfiguration->value("modules").toObject().value("privacy").toBool())
        {
            QDomElement feature15 = document.createElement("feature");
            feature15.setAttribute("var", "jabber:iq:privacy");

            query.appendChild(feature15);
        }

        if (m_serverConfiguration->value("modules").toObject().value("blockingcmd").toBool())
        {
            QDomElement feature16 = document.createElement("feature");
            feature16.setAttribute("var", "urn:xmpp:blocking");

            query.appendChild(feature16);
        }

        if (m_serverConfiguration->value("modules").toObject().value("muc").toBool())
        {
            QDomElement feature17 = document.createElement("feature");
            feature17.setAttribute("var", "http://jabber.org/protocol/muc");

            query.appendChild(feature17);
        }

        if (m_serverConfiguration->value("modules").toObject().value("pubsub").toBool())
        {
            QDomElement feature18 = document.createElement("feature");
            feature18.setAttribute("var", "http://jabber.org/protocol/pubsub");
            query.appendChild(feature18);

            QDomElement feature19 = document.createElement("feature");
            feature19.setAttribute("var", "http://jabber.org/protocol/pubsub#last-published");
            query.appendChild(feature19);

            QDomElement feature20 = document.createElement("feature");
            feature20.setAttribute("var", "http://jabber.org/protocol/pubsub#subscription-options");
            query.appendChild(feature20);
        }

        iq.appendChild(query);
        document.appendChild(iq);

        // Request Acknowledgment of receipt
        // sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    // Entity Queries Chat Service for MUC Support via Disco
    else
    {
        QJsonObject chatServiceObject = serviceExist(to);
        if (!chatServiceObject.isEmpty())
        {
            QDomDocument document;

            QDomElement iq = document.createElement("iq");
            iq.setAttribute("from", to);
            iq.setAttribute("to", from);
            iq.setAttribute("id", id);
            iq.setAttribute("type", "result");

            QDomElement query = document.createElement("query");
            query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");

            QDomElement identity = document.createElement("identity");
            identity.setAttribute("category", chatServiceObject.value("category").toString());
            identity.setAttribute("type", chatServiceObject.value("type").toString());
            identity.setAttribute("name", chatServiceObject.value("name").toString());

            QDomElement feature = document.createElement("feature");
            if (chatServiceObject.value("category").toString() == "conference")
                feature.setAttribute("var", "http://jabber.org/protocol/muc");
            else if (chatServiceObject.value("category").toString() == "proxy")
                feature.setAttribute("var", "http://jabber.org/protocol/bytestreams");

            query.appendChild(identity);
            query.appendChild(feature);

            iq.appendChild(query);
            document.appendChild(iq);
            return document.toByteArray();
        }
        // Entity Queries for Information about a Specific Chat Room
        else if (m_mucManager->chatRoomExist(to))
        {
            QDomDocument document;

            QDomElement iq = document.createElement("iq");
            iq.setAttribute("from", to);
            iq.setAttribute("to", from);
            iq.setAttribute("id", id);
            iq.setAttribute("type", "result");

            QDomElement query = document.createElement("query");
            query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");

            QDomElement identity = document.createElement("identity");
            identity.setAttribute("category", "conference");
            identity.setAttribute("type", "text");
            identity.setAttribute("name", "Qjabber conference service");

            QDomElement feature = document.createElement("feature");
            feature.setAttribute("var", "http://jabber.org/protocol/muc");

            QDomElement feature1 = document.createElement("feature");
            feature1.setAttribute("var", "muc_passwordprotected");

            QDomElement feature2 = document.createElement("feature");
            feature2.setAttribute("var", "muc_hidden");

            QDomElement feature3 = document.createElement("feature");
            feature3.setAttribute("var", "muc_temporary");

            QDomElement feature4 = document.createElement("feature");
            feature4.setAttribute("var", "muc_open");

            QDomElement feature5 = document.createElement("feature");
            feature5.setAttribute("var", "muc_modereted");

            QDomElement feature6 = document.createElement("feature");
            feature6.setAttribute("var", "muc_unmoderated");

            QDomElement feature7 = document.createElement("feature");
            feature7.setAttribute("var", "muc_semianonymous");

            QDomElement feature8 = document.createElement("feature");
            feature8.setAttribute("var", "muc_nonanonymous");

            QDomElement feature9 = document.createElement("feature");
            feature9.setAttribute("var", "muc_public");

            QDomElement feature10 = document.createElement("feature");
            feature10.setAttribute("var", "muc_membersonly");

            QDomElement feature11 = document.createElement("feature");
            feature11.setAttribute("var", "muc_persistent");

            QDomElement feature12 = document.createElement("feature");
            feature12.setAttribute("var", "muc_unsecured");

            QDomElement feature13 = document.createElement("feature");
            feature13.setAttribute("var", "http://jabber.org/protocol/muc#roominfo");

            QDomElement feature14 = document.createElement("feature");
            feature14.setAttribute("var", "http://jabber.org/protocol/muc#roomconfig");

            query.appendChild(identity);
            query.appendChild(feature);
            query.appendChild(feature1);
            query.appendChild(feature2);
            query.appendChild(feature3);
            query.appendChild(feature4);
            query.appendChild(feature5);
            query.appendChild(feature6);
            query.appendChild(feature7);
            query.appendChild(feature8);
            query.appendChild(feature9);
            query.appendChild(feature10);
            query.appendChild(feature11);
            query.appendChild(feature12);
            query.appendChild(feature13);
            query.appendChild(feature14);

            // Extended Disco Info Result
            QDomElement xElement = DataFormManager::getRoomConfigForm("result", "", "", "", m_mucManager->getRoomConfig(to)).firstChildElement().firstChildElement();

            query.appendChild(xElement);
            iq.appendChild(query);
            document.appendChild(iq);

            // Request Acknowledgment of receipt
            // sigSendReceiptRequest(from, document.toByteArray());
            return document.toByteArray();
        }
        // disco#info to an account of virtual host entity
        else if (m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(Utils::getHost(to)))
        {
            if (m_userManager->userExists(Utils::getBareJid(to)))
            {
                QDomDocument document;

                QDomElement iq = document.createElement("iq");
                iq.setAttribute("from", to);
                iq.setAttribute("to", from);
                iq.setAttribute("id", id);
                iq.setAttribute("type", "result");

                QDomElement query = document.createElement("query");
                query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");

                QDomElement identity = document.createElement("identity");
                identity.setAttribute("category", "account");
                identity.setAttribute("type", "registered");

                query.appendChild(identity);
                iq.appendChild(query);
                document.appendChild(iq);

                // Request Acknowledgment of receipt
                // sigSendReceiptRequest(from, document.toByteArray());
                return document.toByteArray();
            }
            else
            {
                return Error::generateError("", "iq", "cancel", "item-not-found", to, from, id, document.documentElement());
            }
        }
        else
        {
            return Error::generateError("", "iq", "cancel", "item-not-found", from, to, id, document.firstChildElement().firstChildElement());
        }
    }
    return QByteArray();
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerItemsQueryResult(QString from, QString to,
                                                                            QString id)
{
    if (m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(to))
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");
        iq.setAttribute("from", to);
        iq.setAttribute("to", from);
        iq.setAttribute("id", id);
        iq.setAttribute("type", "result");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");

        QJsonArray serviceArray = m_serverConfiguration->value("services").toObject().value(to).toArray();
        for (int i = 0; i < serviceArray.count(); ++i)
        {
            QDomElement serviceElement = document.createElement("item");
            serviceElement.setAttribute("jid", serviceArray[i].toObject().value("jid").toString());
            serviceElement.setAttribute("name", serviceArray[i].toObject().value("name").toString());
            query.appendChild(serviceElement);
        }

        iq.appendChild(query);
        document.appendChild(iq);

        // Request Acknowledgment of receipt
        // sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    else
    {
        QJsonObject serviceObject = serviceExist(to);
        if (!serviceObject.isEmpty())
        {
            if (serviceObject.value("category").toString() == "conference")
            {
                QDomDocument document;
                QDomElement iq = document.createElement("iq");
                iq.setAttribute("from", to);
                iq.setAttribute("to", from);
                iq.setAttribute("id", id);
                iq.setAttribute("type", "result");

                QDomElement query = document.createElement("query");
                query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");

                QMultiHash<QString, QString> roomList = m_mucManager->getChatRoomNameList(to);
                foreach (QString key, roomList.keys())
                {
                    QDomElement item = document.createElement("item");
                    item.setAttribute("jid", key);
                    item.setAttribute("name", roomList.value(key));
                    query.appendChild(item);
                }

                iq.appendChild(query);
                document.appendChild(iq);

                // TODO : Result set management in the iq result.

                // Request Acknowledgment of receipt
                // sigSendReceiptRequest(from, document.toByteArray());
                return document.toByteArray();
            }
            // Entity Queries for Items Associated with a Specific Chat Room
            else if (m_mucManager->chatRoomExist(to))
            {
                QDomDocument document;

                QDomElement iq = document.createElement("iq");
                iq.setAttribute("from", to);
                iq.setAttribute("to", from);
                iq.setAttribute("id", id);
                iq.setAttribute("type", "result");

                QDomElement query = document.createElement("query");
                query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");

                //        if (!m_mucManager->isPrivateOccupantsList(to))
                //        {
                QList<QString> occupantsMucJid = m_mucManager->getOccupantsMucJid(to);
                foreach (QString MucJid, occupantsMucJid)
                {
                    QDomElement mucItem = document.createElement("item");
                    mucItem.setAttribute("jid", MucJid);
                    query.appendChild(mucItem);
                }
                //        }
                iq.appendChild(query);
                document.appendChild(iq);

                // Request Acknowledgment of receipt
                // sigSendReceiptRequest(from, document.toByteArray());
                return document.toByteArray();
            }
            else if (m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(Utils::getHost(to)))
            {
                if (!to.contains("/"))
                {
                    emit sigAccountAvailableResourceQuery(from, to, id);
                    return QByteArray();
                }
            }
        }
        return QByteArray();
    }
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerInfoQueryResult(QString iqFrom, QString node,
                                                                           QDomDocument document)
{
    if (node == "http://jabber.org/protocol/offline")
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");
        query.setAttribute("node", document.firstChildElement().attribute("node"));

        QDomElement identity = document.createElement("identity");
        identity.setAttribute("category", "automation");
        identity.setAttribute("type", "message-list");

        QDomElement feature = document.createElement("feature");
        feature.setAttribute("var", "http://jabber.org/protocol/offline");

        QDomElement xNode = document.createElement("x");
        xNode.setAttribute("xmlns", "jabber:x:data");
        xNode.setAttribute("type", "result");

        QDomElement field1 = document.createElement("field");
        field1.setAttribute("type", "hidden");
        field1.setAttribute("var", "FORM_TYPE");

        QDomElement value1 = document.createElement("value");
        value1.appendChild(document.createTextNode("http://jabber.org/protocol/offline"));
        field1.appendChild(value1);

        QDomElement field2 = document.createElement("field");
        field2.setAttribute("var", "number-of-messages");
        field2.setAttribute("label", "username");

        QDomElement value2 = document.createElement("value");
        value2.appendChild(document.createTextNode(QString::number(getOfflineMessagesNumber(Utils::getBareJid(iqFrom)))));
        field2.appendChild(value2);

        xNode.appendChild(field1);
        xNode.appendChild(field2);

        query.appendChild(identity);
        query.appendChild(feature);
        query.appendChild(xNode);
        iq.appendChild(query);

        document.appendChild(iq);

        return document.toByteArray();
    }
    else if (node.at(node.count() - 1) == '=')
    {
        QString to = document.firstChildElement().attribute("to");

        if (m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(Utils::getHost(to)))
        {
            emit sigClientServiceDiscoveryQuery(to, document.toByteArray());
        }
        else if (m_mucManager->chatRoomExist(Utils::getBareJid(to)))
        {
            QString jid = m_mucManager->getOccupantJid(Utils::getBareJid(to), to);
            emit sigClientServiceDiscoveryQuery(jid, document.toByteArray());
        }
        return QByteArray();
    }
    else
    {
        QJsonObject serviceObject = serviceExist(document.documentElement().attribute("to"));
        if (serviceObject.value("category").toString() == "pubsub")
        {
            QDomDocument resultDocument;
            QDomElement iq = resultDocument.createElement("iq");
            iq.setAttribute("from", document.documentElement().attribute("to"));
            iq.setAttribute("to", document.documentElement().attribute("from"));
            iq.setAttribute("id", document.documentElement().attribute("id"));

            QDomElement queryElement = document.documentElement().firstChildElement();
            QDomElement identityElement = document.createElement("identity");
            identityElement.setAttribute("category", "pubsub");
            identityElement.setAttribute("type", "leaf");
            queryElement.appendChild(identityElement);

            iq.appendChild(queryElement);
            resultDocument.appendChild(iq);

            return resultDocument.toByteArray();
        }
    }
    return QByteArray();
}


QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerItemsQueryResult(QString node, QString iqFrom,
                                                                            QDomDocument document)
{
    QString to = document.firstChildElement().attribute("to");
    if (node == "http://jabber.org/protocol/offline")
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");
        query.setAttribute("node", node);

        QMultiHash<QString, QString> offlineMessageHeaders = getOfflineMessageHeaders(iqFrom);

        QList<QString> keyList = offlineMessageHeaders.keys();
        foreach (QString key, keyList)
        {
            QDomElement item = document.createElement("item");
            item.setAttribute("jid", iqFrom);
            item.setAttribute("node", key);
            item.setAttribute("name", offlineMessageHeaders.value(key));
            query.appendChild(item);
        }
        iq.appendChild(query);
        document.appendChild(iq);

        return document.toByteArray();
    }
    else if (node == "http://jabber.org/protocol/muc#rooms")
    {
        emit sigClientServiceDiscoveryQuery(to, document.toByteArray());
        return QByteArray();
    }
    else
    {
        QJsonObject serviceObject = serviceExist(to);
        if (serviceObject.value("category").toString() == "pubsub")
        {
            QList<PubsubItem> nodeItems = m_pubsubManager->getNodeItems(to, node);
            QDomDocument document;
            QDomElement iq = document.createElement("iq");
            iq.setAttribute("from", to);
            iq.setAttribute("to", iqFrom);
            iq.setAttribute("id", document.firstChildElement().attribute("id"));
            iq.setAttribute("type", "result");

            QDomElement query = document.createElement("query");
            query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");

            foreach (PubsubItem nodeItem, nodeItems)
            {
                QDomElement item = document.createElement("item");
                item.setAttribute("jid", to);
                item.setAttribute("name", nodeItem.id());
                query.appendChild(item);
            }

            iq.appendChild(query);
            document.appendChild(iq);

            // TODO : Result set management in the iq result.

            // Request Acknowledgment of receipt
            // sigSendReceiptRequest(from, document.toByteArray());
            return document.toByteArray();
        }
    }
    return QByteArray();
}

int ServiceDiscoveryManager::getOfflineMessagesNumber(QString jid)
{
    return m_userManager->getStorageManager()->getStorage()->getOfflineMessagesNumber(jid);
}

QMultiHash<QString, QString> ServiceDiscoveryManager::getOfflineMessageHeaders(QString jid)
{
    return m_userManager->getStorageManager()->getStorage()->getOfflineMessageHeaders(jid);
}

QJsonObject ServiceDiscoveryManager::serviceExist(QString serviceJid)
{
    foreach (QString host, m_serverConfiguration->value("services").toVariant().toMap().keys())
    {
        QJsonArray hostServiceList = m_serverConfiguration->value("services").toObject().value(host).toArray();
        for (int i = 0; i < hostServiceList.count(); ++i)
        {
            if (hostServiceList[i].toObject().value("jid") == serviceJid)
                return hostServiceList[i].toObject();
        }
    }
    return QJsonObject();
}
