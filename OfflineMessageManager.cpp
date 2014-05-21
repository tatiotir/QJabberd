#include "OfflineMessageManager.h"

OfflineMessageManager::OfflineMessageManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
}

/**
 * Offline message reply
 *
 * @brief OfflineMessageManager::offlineMessageManagerReply
 * @param iqXML
 * @param iqFrom
 * @return QByteArray
 */
QByteArray OfflineMessageManager::offlineMessageManagerReply(QByteArray iqXML, QString iqFrom)
{
    QDomDocument document;
    document.setContent(iqXML);

    QString id = document.documentElement().attribute("id", Utils::generateId());
    QString offlineFirstChildTagName = document.documentElement().firstChildElement().firstChildElement().tagName();
    if (offlineFirstChildTagName == "fetch")
    {
        QMultiHash<QString, QByteArray> messageList = getAllOfflineMessage(Utils::getBareJid(iqFrom));
        QList<QString> keyList = messageList.keys();

        QByteArray allMessages;
        foreach (QString key, keyList)
        {
            QDomDocument document;
            document.setContent(messageList.value(key));

            QDomElement offlineElement = document.createElement("offline");
            offlineElement.setAttribute("xmlns", "http://jabber.org/protocol/offline");

            QDomElement item = document.createElement("item");
            item.setAttribute("node", key);

            offlineElement.appendChild(item);
            document.documentElement().appendChild(offlineElement);
            allMessages += document.toByteArray();
        }
        return offlineMessageManagerResult(iqFrom, id, allMessages);
    }
    else if (offlineFirstChildTagName == "purge")
    {
        deleteAllOfflineMessage(Utils::getBareJid(iqFrom));
        return offlineMessageManagerResult(iqFrom, id, QByteArray());
    }
    else
    {
        QDomNodeList itemList = document.documentElement().elementsByTagName("item");
        for (int i = 0; i < itemList.count(); ++i)
        {
            QString action = itemList.item(i).toElement().attribute("action");
            QString node = itemList.item(i).toElement().attribute("node");
            QString name = itemList.item(i).toElement().attribute("name");

            if (action == "view")
            {
                if (!node.isEmpty())
                {
                    QByteArray message = getOfflineMessage(Utils::getBareJid(iqFrom), node);
                    QDomDocument document;
                    document.setContent(message);

                    QDomElement offlineElement = document.createElement("offline");
                    offlineElement.setAttribute("xmlns", "http://jabber.org/protocol/offline");

                    QDomElement item = document.createElement("item");
                    item.setAttribute("node", node);

                    offlineElement.appendChild(item);
                    document.documentElement().appendChild(offlineElement);

                    return offlineMessageManagerResult(iqFrom, id, document.toByteArray());
                }

                if (!name.isEmpty())
                {
                    QMultiHash<QString, QByteArray> messageList = getOfflineMessageFrom(Utils::getBareJid(iqFrom), name);
                    QList<QString> keyList = messageList.keys();

                    QByteArray allMessages;
                    foreach (QString key, keyList)
                    {
                        QDomDocument document;
                        document.setContent(messageList.value(key));

                        QDomElement offlineElement = document.createElement("offline");
                        offlineElement.setAttribute("xmlns", "http://jabber.org/protocol/offline");

                        QDomElement item = document.createElement("item");
                        item.setAttribute("name", name);
                        item.setAttribute("node", key);

                        offlineElement.appendChild(item);
                        document.documentElement().appendChild(offlineElement);
                        allMessages += document.toByteArray();
                    }
                    return offlineMessageManagerResult(iqFrom, id, allMessages);
                }
            }
            else if (action == "remove")
            {
                if (!node.isEmpty())
                {
                    deleteOfflineMessage(Utils::getBareJid(iqFrom), node);
                    return offlineMessageManagerResult(iqFrom, id, QByteArray());
                }

                if (!name.isEmpty())
                {
                    deleteOfflineMessage(Utils::getBareJid(iqFrom), name);
                    return offlineMessageManagerResult(iqFrom, id, QByteArray());
                }
            }
        }
    }
}

/**
 * Offline message retrival result
 *
 * @brief OfflineMessageManager::offlineMessageManagerResult
 * @param to
 * @param id
 * @param messageList
 * @return QByteArray
 */
QByteArray OfflineMessageManager::offlineMessageManagerResult(QString to, QString id, QByteArray messageList)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);
    document.appendChild(iq);

    return (messageList.isEmpty() ? (document.toByteArray()) : (messageList + document.toByteArray()));
}

/**
 * Get offline message from a user
 *
 * @brief OfflineMessageManager::getOfflineMessageFrom
 * @param jid
 * @param from
 * @return QMultiHash<QString, QByteArray>
 */
QMultiHash<QString, QByteArray> OfflineMessageManager::getOfflineMessageFrom(QString jid, QString from)
{
    return m_storageManager->getStorage()->getOfflineMessageFrom(jid, from);
}

/**
 * Get offline message using the time stamp at which the message has been sent
 *
 * @brief OfflineMessageManager::getOfflineMessage
 * @param jid
 * @param stamp
 * @return QByteArray
 */
QByteArray OfflineMessageManager::getOfflineMessage(QString jid, QString stamp)
{
    return m_storageManager->getStorage()->getOfflineMessage(jid, stamp);
}

/**
 * Get all offline message for a user
 *
 * @brief OfflineMessageManager::getAllOfflineMessage
 * @param jid
 * @return QMultiHash<QString, QByteArray>
 */
QMultiHash<QString, QByteArray> OfflineMessageManager::getAllOfflineMessage(QString jid)
{
    return m_storageManager->getStorage()->getAllOfflineMessage(jid);
}

/**
 * Delete offline message using the key
 *
 * @brief OfflineMessageManager::deleteOfflineMessage
 * @param jid
 * @param key
 */
void OfflineMessageManager::deleteOfflineMessage(QString jid, QString key)
{
    m_storageManager->getStorage()->deleteOfflineMessage(jid, key);
}

void OfflineMessageManager::deleteAllOfflineMessage(QString jid)
{
    m_storageManager->getStorage()->deleteAllOfflineMessage(jid);
}

