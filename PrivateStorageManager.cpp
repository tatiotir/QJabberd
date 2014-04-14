#include "PrivateStorageManager.h"

PrivateStorageManager::PrivateStorageManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
}

QByteArray PrivateStorageManager::privateStorageManagerReply(QByteArray iqXML, QString from)
{
    QDomDocument document;
    document.setContent(iqXML);

    QDomElement iq = document.documentElement();
    QString id = document.documentElement().attribute("id");
    QString iqType = document.documentElement().attribute("type");
    QString node = iq.firstChildElement().firstChildElement().attribute("xmlns");

    if ((node.indexOf("http://jabber.org") == 0) || (node.indexOf("jabber:") == 0) || (node == "vcard-temp"))
    {
        return Error::generateError("iq", "modify", "not-acceptable", "", "", id, iq.firstChildElement());
    }

    if (iqType == "get")
    {
        if (node == "storage:metacontacts")
        {
            QByteArray dataResult = privateStorageManagerResult(getPrivateData(Utils::getBareJid(from)), id);
            // Request Acknowledgment of receipt
            sigSendReceiptRequest(from, dataResult);
            return dataResult;
        }
        else
        {
            QByteArray dataResult = privateStorageManagerResult(getPrivateData(Utils::getBareJid(from), node), id);;
            // Request Acknowledgment of receipt
            sigSendReceiptRequest(from, dataResult);
            return dataResult;
        }
    }
    else if (iqType == "set")
    {
        if (!iq.attribute("to").isEmpty())
        {
            return Error::generateError("iq", "auth", "forbidden", "", "", id, iq.firstChildElement());
        }

        if (iq.firstChildElement().childNodes().isEmpty())
        {
            return Error::generateError("iq", "cancel", "bad-format", "", "", id, iq.firstChildElement());
        }

        if (node == "storage:metacontacts")
        {
            QDomNodeList metaChildren = iq.firstChildElement().elementsByTagName("meta");

            QList<MetaContact> metaContactList;
            for (int i = 0; i < metaChildren.count(); ++i)
            {
                QDomElement metaChild = metaChildren.item(i).toElement();
                metaContactList << MetaContact(metaChild.attribute("jid"), metaChild.attribute("tab"),
                                               metaChild.attribute("order").toInt());
            }
            storePrivateData(Utils::getBareJid(from), metaContactList);
        }
        else
        {
            QString nodeData;
            QTextStream stream(&nodeData);

            QDomNodeList nodeList = document.documentElement().firstChildElement().childNodes();
            QMultiHash<QString, QString> nodeMap;
            for (int i = 0; i < nodeList.count(); ++i)
            {
                nodeList.item(i).save(stream, 4);
                nodeMap.insert(nodeList.item(i).toElement().attribute("xmlns"), nodeData);
                nodeData.clear();
            }
            storePrivateData(Utils::getBareJid(from), nodeMap);
        }

        QDomDocument document;
        QDomElement iq = document.createElement("iq");

        iq.setAttribute("type", "result");
        iq.setAttribute("id", id);
        document.appendChild(iq);

        // Request Acknowledgment of receipt
        sigSendReceiptRequest(from, document.toByteArray());
        return document.toByteArray();
    }
    return QByteArray();
}

QByteArray PrivateStorageManager::privateStorageManagerResult(QList<MetaContact> metacontactList, QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");
    iq.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:private");

    QDomElement storage = document.createElement("storage");
    storage.setAttribute("xmlns", "storage:metacontacts");

    foreach (MetaContact metaContact, metacontactList)
    {
        QDomElement meta = document.createElement("meta");
        meta.setAttribute("jid", metaContact.getJid());
        meta.setAttribute("tag", metaContact.getTag());
        meta.setAttribute("order", metaContact.getOrder());

        storage.appendChild(meta);
    }

    query.appendChild(storage);
    iq.appendChild(query);
    document.appendChild(iq);

    return document.toByteArray();
}

QByteArray PrivateStorageManager::privateStorageManagerResult( QByteArray data, QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");
    iq.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:private");

    QDomDocument nodeDocument;
    nodeDocument.setContent(data);

    query.appendChild(nodeDocument.documentElement());
    iq.appendChild(query);
    document.appendChild(iq);

    return document.toByteArray();
}

QList<MetaContact> PrivateStorageManager::getPrivateData(QString jid)
{
    return m_storageManager->getStorage()->getPrivateData(jid);
}

QByteArray PrivateStorageManager::getPrivateData(QString jid, QString node)
{
    return m_storageManager->getStorage()->getPrivateData(jid, node);
}

bool PrivateStorageManager::storePrivateData(QString jid, QList<MetaContact> metaContactList)
{
    return m_storageManager->getStorage()->storePrivateData(jid, metaContactList);
}

bool PrivateStorageManager::storePrivateData(QString jid, QMultiHash<QString, QString> nodeMap)
{
    return m_storageManager->getStorage()->storePrivateData(jid, nodeMap);
}
