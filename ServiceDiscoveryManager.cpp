#include "ServiceDiscoveryManager.h"

ServiceDiscoveryManager::ServiceDiscoveryManager(UserManager *userManager)
{
    m_userManager = userManager;
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerReply(QByteArray iqXML, QString iqFrom)
{
    QDomDocument document;
    document.setContent(iqXML);

    QDomElement iq = document.documentElement();
    QString to = iq.attribute("to");
    QString from = iq.attribute("from", iqFrom);
    QString xmlns = iq.firstChildElement().attribute("xmlns");
    QString id = iq.attribute("id", Utils::generateId());

    if (xmlns == "http://jabber.org/protocol/disco#items")
    {
        QString node = iq.firstChildElement().attribute("node");
        if (!node.isEmpty())
        {
            return serviceDiscoveryManagerItemsQueryResult(node, Utils::getBareJid(iqFrom));
        }
        else
        {
            return serviceDiscoveryManagerItemsQueryResult(from, to, id, iq.firstChildElement());
        }
    }
    else if (xmlns == "http://jabber.org/protocol/disco#info")
    {
        QString node = iq.firstChildElement().attribute("node");
        if (!node.isEmpty())
        {
            return serviceDiscoveryManagerInfoQueryResult(iqFrom, iq);
        }
        else
        {
            return serviceDiscoveryManagerInfoQueryResult(from, to, id, iq.firstChildElement());
        }
    }
    return QByteArray();
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerInfoQueryResult(QString from, QString to, QString id,
                                                                           QDomElement request)
{
    if (to == "localhost")
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

        QDomElement feature1 = document.createElement("feature");
        feature1.setAttribute("var", "http://jabber.org/protocol/disco#info");

        QDomElement feature2 = document.createElement("feature");
        feature2.setAttribute("var", "jabber:iq:register");

        QDomElement feature3 = document.createElement("feature");
        feature3.setAttribute("var", "urn:xmpp:ping");

        QDomElement feature4 = document.createElement("feature");
        feature4.setAttribute("var", "vcard-temp");

        QDomElement feature5 = document.createElement("feature");
        feature5.setAttribute("var", "jabber:iq:private");

        QDomElement feature6 = document.createElement("feature");
        feature6.setAttribute("var", "jabber:iq:last");

        QDomElement feature7 = document.createElement("feature");
        feature7.setAttribute("var", "jabber:iq:roster");

        QDomElement feature8 = document.createElement("feature");
        feature8.setAttribute("var", "urn:xmpp:time");

        QDomElement feature9 = document.createElement("feature");
        feature9.setAttribute("var", "http://jabber.org/protocol/offline");

        QDomElement feature10 = document.createElement("feature");
        feature10.setAttribute("var", "msgoffline");

        QDomElement feature11 = document.createElement("feature");
        feature11.setAttribute("var", "http://jabber.org/protocol/disco#items");

        query.appendChild(identity1);
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

        iq.appendChild(query);
        document.appendChild(iq);

        // Request Acknowledgment of receipt
        sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    else if (to == "conference.localhost")
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

        query.appendChild(identity);
        query.appendChild(feature);

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
    else if (Utils::getHost(to) == "localhost")
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
            return Error::generateError("iq", "cancel", "item-not-found", to, from, id, request);
        }
    }
    return QByteArray();
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerItemsQueryResult(QString from, QString to, QString id,
                                                                            QDomElement request)
{
    if (to == "localhost")
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");
        iq.setAttribute("from", to);
        iq.setAttribute("to", from);
        iq.setAttribute("id", id);
        iq.setAttribute("type", "result");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");

        QDomElement item1 = document.createElement("item");
        item1.setAttribute("jid", "conference.localhost");
        item1.setAttribute("name", "QJabberd chatroom service");

        query.appendChild(item1);

        iq.appendChild(query);
        document.appendChild(iq);


        // Request Acknowledgment of receipt
        sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    else if (to == "conference.localhost")
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");
        iq.setAttribute("from", to);
        iq.setAttribute("to", from);
        iq.setAttribute("id", id);
        iq.setAttribute("type", "result");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");

        QMultiHash<QString, QString> roomList;
        QList<QString> keys = roomList.keys();

        foreach (QString key, keys)
        {
            QDomElement item = document.createElement("item");
            item.setAttribute("jid", roomList.value(key));
            item.setAttribute("name", key);

            query.appendChild(item);
        }

        iq.appendChild(query);
        document.appendChild(iq);

        // TODO : Result set management in the iq result.

        // Request Acknowledgment of receipt
        sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    else if (Utils::getHost(to) == "localhost")
    {
        if (!to.contains("/"))
        {
            emit sigAccountAvailableResourceQuery(from, to, id);
            return QByteArray();
        }
    }
    return QByteArray();
}

QByteArray ServiceDiscoveryManager::serviceDiscoveryManagerInfoQueryResult(QString iqFrom, QDomElement iqElement)
{
    if (iqElement.firstChildElement().attribute("node") == "http://jabber.org/protocol/offline")
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");
        query.setAttribute("node", iqElement.firstChildElement().attribute("node"));

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
    else
    {
        QDomDocument document;
        QDomElement iq = document.createElement("iq");
        iq = iqElement;
        iq.setAttribute("from", iqFrom);
        document.appendChild(iq);

        qDebug() << "to : " << iqElement.attribute("to");

        emit sigClientServiceDiscoveryQuery(iqElement.attribute("to"), document.toByteArray());
        return QByteArray();
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
