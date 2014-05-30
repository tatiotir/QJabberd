#include "ServiceDiscoveryManager.h"

ServiceDiscoveryManager::ServiceDiscoveryManager(QMap<QString, QVariant> *serverConfigMap,
                                                 UserManager *userManager, MucManager *mucManager)
{
    m_serverConfigMap = serverConfigMap;
    m_userManager = userManager;
    m_mucManager = mucManager;
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
        if (to.contains("/"))
        {
            if (type == "set")
            {
                emit sigClientServiceDiscoveryQuery(to, document.toByteArray());
            }
            else if (type == "result")
            {
                emit sigClientServiceDiscoveryResponse(to, document.toByteArray());
            }
            return QByteArray();
        }

        QString node = iq.firstChildElement().attribute("node");
        if (!node.isEmpty())
        {
            return serviceDiscoveryManagerItemsQueryResult(node, Utils::getBareJid(iqFrom));
        }
        else
        {
            return serviceDiscoveryManagerItemsQueryResult(from, to, id, document);
        }
    }
    else if (xmlns == "http://jabber.org/protocol/disco#info")
    {
        if (to.contains("/"))
        {
            if (type == "set")
            {
                emit sigClientServiceDiscoveryQuery(to, document.toByteArray());
            }
            else if (type == "result")
            {
                emit sigClientServiceDiscoveryResponse(to, document.toByteArray());
            }
            return QByteArray();
        }

        QString node = iq.firstChildElement().attribute("node");
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
    if (m_serverConfigMap->value("virtualHost").toList().contains(to))
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
        identity1.setAttribute("name", "Qjabber");

        query.appendChild(identity1);

        if (m_serverConfigMap->value("modules").toMap().value("disco").toBool())
        {
            QDomElement feature1 = document.createElement("feature");
            feature1.setAttribute("var", "http://jabber.org/protocol/disco#info");

            QDomElement feature2 = document.createElement("feature");
            feature2.setAttribute("var", "http://jabber.org/protocol/disco#items");

            query.appendChild(feature1);
            query.appendChild(feature2);
        }

        if (m_serverConfigMap->value("modules").toMap().value("register").toBool())
        {
            QDomElement feature3 = document.createElement("feature");
            feature3.setAttribute("var", "jabber:iq:register");

            query.appendChild(feature3);
        }

        if (m_serverConfigMap->value("modules").toMap().value("ping").toBool())
        {
            QDomElement feature4 = document.createElement("feature");
            feature4.setAttribute("var", "urn:xmpp:ping");

            query.appendChild(feature4);
        }

        if (m_serverConfigMap->value("modules").toMap().value("vcard-temp").toBool())
        {
            QDomElement feature5 = document.createElement("feature");
            feature5.setAttribute("var", "vcard-temp");

            query.appendChild(feature5);
        }

        if (m_serverConfigMap->value("modules").toMap().value("private").toBool())
        {
            QDomElement feature6 = document.createElement("feature");
            feature6.setAttribute("var", "jabber:iq:private");

            query.appendChild(feature6);
        }

        if (m_serverConfigMap->value("modules").toMap().value("lastActivity").toBool())
        {
            QDomElement feature7 = document.createElement("feature");
            feature7.setAttribute("var", "jabber:iq:last");

            query.appendChild(feature7);
        }

        if (m_serverConfigMap->value("modules").toMap().value("roster").toBool())
        {
            QDomElement feature8 = document.createElement("feature");
            feature8.setAttribute("var", "jabber:iq:roster");

            query.appendChild(feature8);
        }

        if (m_serverConfigMap->value("modules").toMap().value("time").toBool())
        {
            QDomElement feature9 = document.createElement("feature");
            feature9.setAttribute("var", "urn:xmpp:time");

            query.appendChild(feature9);
        }

        if (m_serverConfigMap->value("modules").toMap().value("offline").toBool())
        {
            QDomElement feature10 = document.createElement("feature");
            feature10.setAttribute("var", "http://jabber.org/protocol/offline");

            query.appendChild(feature10);
        }

        if (m_serverConfigMap->value("modules").toMap().value("msgoffline").toBool())
        {
            QDomElement feature11 = document.createElement("feature");
            feature11.setAttribute("var", "msgoffline");

            query.appendChild(feature11);
        }

        if (m_serverConfigMap->value("modules").toMap().value("nonsaslauth").toBool())
        {
            QDomElement feature12 = document.createElement("feature");
            feature12.setAttribute("var", "jabber:iq:auth");

            query.appendChild(feature12);
        }

        if (m_serverConfigMap->value("modules").toMap().value("si").toBool())
        {
            QDomElement feature13 = document.createElement("feature");
            feature13.setAttribute("var", "http://jabber.org/protocol/si");

            query.appendChild(feature13);
        }

        if (m_serverConfigMap->value("modules").toMap().value("file-transfert").toBool())
        {
            QDomElement feature14 = document.createElement("feature");
            feature14.setAttribute("var", "http://jabber.org/protocol/file-transfert");

            query.appendChild(feature14);
        }

        if (m_serverConfigMap->value("modules").toMap().value("ibb").toBool())
        {
            QDomElement feature13 = document.createElement("feature");
            feature13.setAttribute("var", "http://jabber.org/protocol/ibb");

            query.appendChild(feature13);
        }

        if (m_serverConfigMap->value("modules").toMap().value("oob").toBool())
        {
            QDomElement feature14 = document.createElement("feature");
            feature14.setAttribute("var", "http://jabber.org/protocol/oob");

            query.appendChild(feature14);
        }

        if (m_serverConfigMap->value("modules").toMap().value("privacy").toBool())
        {
            QDomElement feature15 = document.createElement("feature");
            feature15.setAttribute("var", "jabber:iq:privacy");

            query.appendChild(feature15);
        }

        if (m_serverConfigMap->value("modules").toMap().value("blockingcmd").toBool())
        {
            QDomElement feature16 = document.createElement("feature");
            feature16.setAttribute("var", "urn:xmpp:blocking");

            query.appendChild(feature16);
        }

        iq.appendChild(query);
        document.appendChild(iq);

        // Request Acknowledgment of receipt
        sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    // Entity Queries Chat Service for MUC Support via Disco
    else if (m_serverConfigMap->value("muc").toStringList().contains(to))
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
        identity.setAttribute("name", "Chat room");

        QDomElement feature = document.createElement("feature");
        feature.setAttribute("var", "http://jabber.org/protocol/muc");

        query.appendChild(identity);
        query.appendChild(feature);

        document.appendChild(query);
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

        iq.appendChild(query);
        document.appendChild(iq);

        // Request Acknowledgment of receipt
        sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    else if (Utils::getHost(to) == "conference.localhost")
    {

    }
    // disco#info to an account of virtual host entity
    else if (m_serverConfigMap->value("virtualHost").toList().contains(Utils::getHost(to)))
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
            sigSendReceiptRequest(from, document.toByteArray());
            return document.toByteArray();
        }
        else
        {
            return Error::generateError("iq", "cancel", "item-not-found", to, from, id, document.documentElement());
        }
    }
    return QByteArray();
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerItemsQueryResult(QString from, QString to, QString id,
                                                                            QDomDocument document)
{
    if (m_serverConfigMap->value("virtualHost").toList().contains(to))
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");
        iq.setAttribute("from", to);
        iq.setAttribute("to", from);
        iq.setAttribute("id", id);
        iq.setAttribute("type", "result");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");

        foreach (QString muc, m_serverConfigMap->value("muc").toStringList())
        {
            QDomElement mucItem = document.createElement("item");
            mucItem.setAttribute("jid", muc);
            mucItem.setAttribute("name", "Qjabberd chatroom service");
            query.appendChild(mucItem);
        }

        iq.appendChild(query);
        document.appendChild(iq);


        // Request Acknowledgment of receipt
        sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    else if (m_serverConfigMap->value("muc").toStringList().contains(to))
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
        sigSendReceiptRequest(from, document.toByteArray());
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

        if (!m_mucManager->isPrivateOccupantsList(to))
        {
            QList<QString> occupantsList = m_mucManager->getChatRoomOccupants(to);
            foreach (QString jid, occupantsList)
            {
                QDomElement mucItem = document.createElement("item");
                mucItem.setAttribute("jid", jid);
                query.appendChild(mucItem);
            }
        }
        iq.appendChild(query);
        document.appendChild(iq);

        // Request Acknowledgment of receipt
        sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    else if (m_serverConfigMap->value("virtualHost").toStringList().contains(Utils::getHost(to)))
    {
        if (!to.contains("/"))
        {
            emit sigAccountAvailableResourceQuery(from, to, id);
            return QByteArray();
        }
    }
    return QByteArray();
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
    return QByteArray();
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerItemsQueryResult(QString node, QString iqFrom)
{
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
}

int ServiceDiscoveryManager::getOfflineMessagesNumber(QString jid)
{
    return m_userManager->getStorageManager()->getStorage()->getOfflineMessagesNumber(jid);
}


QMultiHash<QString, QString> ServiceDiscoveryManager::getOfflineMessageHeaders(QString jid)
{
    return m_userManager->getStorageManager()->getStorage()->getOfflineMessageHeaders(jid);
}
