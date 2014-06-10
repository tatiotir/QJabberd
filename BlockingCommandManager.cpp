#include "BlockingCommandManager.h"

BlockingCommandManager::BlockingCommandManager(StorageManager *storageManager, RosterManager *rosterManager)
{
    m_storageManager = storageManager;
    m_rosterManager = rosterManager;
}

QByteArray BlockingCommandManager::blockingCommandManagerReply(QDomDocument document, QString iqFrom)
{
    QDomElement iq = document.firstChildElement();

    if (iq.attribute("type") == "get")
    {
        return blockingCommandManagerResult(iq.attribute("id"), getUserBlockList(Utils::getBareJid(iqFrom)));
    }
    else if (iq.attribute("type") == "set")
    {
        if (iq.firstChildElement().tagName() == "block")
        {
            QDomNodeList itemListElement = document.firstChildElement().elementsByTagName("item");
            if (itemListElement.isEmpty())
            {
                return Error::generateError("", "iq", "modify", "bad-request", iqFrom, "", iq.attribute("id"), QDomElement());
            }

            QList<QString> itemList;
            for (int i = 0; i < itemListElement.count(); ++i)
            {
                itemList << itemListElement.item(i).toElement().attribute("jid");
            }

            if (addUserBlockListItems(Utils::getBareJid(iqFrom), itemList))
            {
                foreach (QString item, itemList)
                {
                    emit sigUnavailablePresenceBroadCast(item, Utils::getBareJid(iqFrom));
                }

                emit sigBlockPush(Utils::getBareJid(iqFrom), itemList);
                return generateIQResult(iqFrom, iq.attribute("id"));
            }
            else
            {
                return Error::generateInternalServerError();
            }
        }
        else if (iq.firstChildElement().tagName() == "unblock")
        {
            QDomNodeList itemListElement = document.firstChildElement().elementsByTagName("item");
            if (itemListElement.isEmpty())
            {
                QList<QString> itemList = getUserBlockList(Utils::getBareJid(iqFrom));
                if (emptyUserBlockList(Utils::getBareJid(iqFrom)))
                {
                    foreach (QString item, itemList)
                    {
                        if (m_rosterManager->contactExists(Utils::getBareJid(iqFrom), item))
                            emit sigPresenceBroadCastFromContact(item, Utils::getBareJid(iqFrom));
                    }
                    emit sigUnblockPush(Utils::getBareJid(iqFrom), QList<QString>());
                    return generateIQResult(iqFrom, iq.attribute("id"));
                }
                else
                {
                    return Error::generateInternalServerError();
                }
            }
            QList<QString> itemList;
            for (int i = 0; i < itemListElement.count(); ++i)
            {
                itemList << itemListElement.item(i).toElement().attribute("jid");
            }

            if (deleteUserBlockListItems(Utils::getBareJid(iqFrom), itemList))
            {
                foreach (QString item, itemList)
                {
                    if (m_rosterManager->contactExists(Utils::getBareJid(iqFrom), item))
                        emit sigPresenceBroadCastFromContact(item, Utils::getBareJid(iqFrom));
                }
                emit sigUnblockPush(Utils::getBareJid(iqFrom), itemList);
                return generateIQResult(iqFrom, iq.attribute("id"));
            }
        }
    }
}

QByteArray BlockingCommandManager::generateIQResult(QString to, QString id)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    if (!to.isEmpty())
    {
        iq.setAttribute("to", to);
    }
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    document.appendChild(iq);
    return document.toByteArray();
}

QByteArray BlockingCommandManager::blockingCommandManagerResult(QString id, QList<QString> blocklist)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");
    iq.setAttribute("id", id);

    QDomElement blocklistElement = document.createElement("blocklist");
    blocklistElement.setAttribute("xmlns", "urn:xmpp:blocking");

    foreach (QString jid, blocklist)
    {
        QDomElement item = document.createElement("item");
        item.setAttribute("jid", jid);
        blocklistElement.appendChild(item);
    }
    iq.appendChild(blocklistElement);
    document.appendChild(iq);

    return document.toByteArray();
}

QDomDocument BlockingCommandManager::generateBlockPush(QString to, QString id, QList<QString> items)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("to", to);
    iq.setAttribute("type", "set");
    iq.setAttribute("id", id);

    QDomElement blockElement = document.createElement("block");
    blockElement.setAttribute("xmlns", "urn:xmpp:blocking");

    foreach (QString jid, items)
    {
        QDomElement item = document.createElement("item");
        item.setAttribute("jid", jid);
        blockElement.appendChild(item);
    }
    iq.appendChild(blockElement);
    document.appendChild(iq);

    return document;
}

QDomDocument BlockingCommandManager::generateUnblockPush(QString to, QString id, QList<QString> items)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("to", to);
    iq.setAttribute("type", "set");
    iq.setAttribute("id", id);

    QDomElement blockElement = document.createElement("unblock");
    blockElement.setAttribute("xmlns", "urn:xmpp:blocking");

    if (!items.isEmpty())
    {
        foreach (QString jid, items)
        {
            QDomElement item = document.createElement("item");
            item.setAttribute("jid", jid);
            blockElement.appendChild(item);
        }
    }
    iq.appendChild(blockElement);
    document.appendChild(iq);

    return document;
}

QByteArray BlockingCommandManager::isBlocked(QString from, QString to, QString stanzaType)
{
    QList<QString> fromBlockList = getUserBlockList(Utils::getBareJid(from));
    QList<QString> toBlockList = getUserBlockList(Utils::getBareJid(to));

    // Check block list items
    if (fromBlockList.contains(to) ||
            fromBlockList.contains(Utils::getBareJid(to)) ||
            fromBlockList.contains(Utils::getHost(to)) ||
            fromBlockList.contains(to.split("@").value(1)))
    {
        QDomDocument document;
        QDomElement blockedElement = document.createElement("blocked");
        blockedElement.setAttribute("xmlns", "urn:xmpp:blocking:errors");

        return Error::generateError("", "message", "cancel", "not-acceptable",
                                         from, to, "", blockedElement);
    }
    else if (toBlockList.contains(from) ||
             toBlockList.contains(Utils::getBareJid(from)) ||
             toBlockList.contains(Utils::getHost(from)) ||
             toBlockList.contains(from.split("@").value(1)))
    {
        if (stanzaType != "presence")
        {
            return Error::generateError("", "iq", "cancel", "service-unavalaible",
                                         Utils::getHost(from), from, "", QDomElement());
        }
        else
        {
            return QByteArray("a");
        }
    }
    return QByteArray();
}

QList<QString> BlockingCommandManager::getUserBlockList(QString jid)
{
    return m_storageManager->getStorage()->getUserBlockList(jid);
}

bool BlockingCommandManager::addUserBlockListItems(QString jid, QList<QString> items)
{
    return m_storageManager->getStorage()->addUserBlockListItems(jid, items);
}

bool BlockingCommandManager::deleteUserBlockListItems(QString jid, QList<QString> items)
{
    return m_storageManager->getStorage()->deleteUserBlockListItems(jid, items);
}

bool BlockingCommandManager::emptyUserBlockList(QString jid)
{
    return m_storageManager->getStorage()->emptyUserBlockList(jid);
}
