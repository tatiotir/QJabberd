#include "OfflineMessageManager.h"

OfflineMessageManager::OfflineMessageManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
}

/*!
 * \brief The OfflineMessageManager::offlineMessageManagerReply method parse an offline message query an return response
 * \param iqXML
 * \param iqFrom
 * \return QByteArray
 */
QByteArray OfflineMessageManager::offlineMessageManagerReply(QDomDocument document, QString iqFrom)
{
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

/*!
 * \brief The OfflineMessageManager::offlineMessageManagerResult method return iq result for an offline message query
 * \param to
 * \param id
 * \param messageList
 * \return QByteArray
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

/*!
 * \brief The OfflineMessageManager::getOfflineMessageFrom method get user offline message
 * \param jid
 * \param from
 * \return QMultiHash<QString, QByteArray>
 */
QMultiHash<QString, QByteArray> OfflineMessageManager::getOfflineMessageFrom(QString jid, QString from)
{
    return m_storageManager->getStorage()->getOfflineMessageFrom(jid, from);
}

/*!
 * \brief The OfflineMessageManager::getOfflineMessage method get user offline message using the time stamp at which the message has been sent
 * \param jid
 * \param stamp
 * \return QByteArray
 */
QByteArray OfflineMessageManager::getOfflineMessage(QString jid, QString stamp)
{
    return m_storageManager->getStorage()->getOfflineMessage(jid, stamp);
}

/*!
 * \brief The OfflineMessageManager::getAllOfflineMessage metod get all user offline message
 * \param jid
 * \return QMultiHash<QString, QByteArray>
 */
QMultiHash<QString, QByteArray> OfflineMessageManager::getAllOfflineMessage(QString jid)
{
    return m_storageManager->getStorage()->getAllOfflineMessage(jid);
}

/*!
 * \brief The OfflineMessageManager::deleteOfflineMessage method delete user offline message using the key (stamp or user jid)
 * \param jid
 * \param key
 */
void OfflineMessageManager::deleteOfflineMessage(QString jid, QString key)
{
    m_storageManager->getStorage()->deleteOfflineMessage(jid, key);
}

/*!
 * \brief The OfflineMessageManager::deleteAllOfflineMessage method delete all usser offline message
 * \param jid
 */
void OfflineMessageManager::deleteAllOfflineMessage(QString jid)
{
    m_storageManager->getStorage()->deleteAllOfflineMessage(jid);
}

