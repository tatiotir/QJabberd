#include "BlockingCommandManager.h"

BlockingCommandManager::BlockingCommandManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
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
                return Error::generateError("iq", "modify", "bad-request", iqFrom, "", iq.attribute("id"), QDomElement());
            }
            QList<QString> itemList;
            for (int i = 0; i < itemList.count(); ++i)
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
                if (emptyUserBlockList(Utils::getBareJid(iqFrom)))
                {
                    emit sigUnblockPush(Utils::getBareJid(iqFrom), QList<QString>());
                    return generateIQResult(iqFrom, iq.attribute("id"));
                }
                else
                {
                    return Error::generateInternalServerError();
                }
            }
            QList<QString> itemList;
            for (int i = 0; i < itemList.count(); ++i)
            {
                itemList << itemListElement.item(i).toElement().attribute("jid");
            }

            if (deleteUserBlockListItems(Utils::getBareJid(iqFrom), itemList))
            {
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
