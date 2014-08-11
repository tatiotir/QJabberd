#include "PrivacyListManager.h"

PrivacyListManager::PrivacyListManager(QObject *parent, StorageManager *storageManager,
                                       RosterManager *rosterManager) :
    QObject(parent)
{
    m_storageManager =  storageManager;
    m_rosterMananager = rosterManager;
}

QByteArray PrivacyListManager::privacyListReply(QDomDocument document, QString iqFrom)
{
    QDomElement iq = document.documentElement();
    QString from = iq.attribute("from", iqFrom);
    QString to = iq.attribute("to");
    QString id = iq.attribute("id", Utils::generateId());

    QDomElement query = iq.firstChild().toElement();
    if (iq.attribute("type") == "get")
    {
        if (query.firstChildElement().isNull())
        {
            return privacyListNames(to, from, id, getPrivacyListNames(Utils::getBareJid(from)),
                                    getActivePrivacyList(Utils::getBareJid(from)),
                                    getDefaultPrivacyList(Utils::getBareJid(from)));
        }
        else
        {
            QString privacyListName = query.firstChild().toElement().attribute("name");
            if (!privacyListExist(Utils::getBareJid(from), privacyListName))
            {
                return Error::generateError("", "iq", "cancel", "item-not-found", "", from, id,
                                                   iq.firstChildElement());
            }

            if (query.elementsByTagName("list").count() > 1)
            {
                return Error::generateError("", "iq", "modify", "bad-request", "", from, id, iq.firstChildElement());
            }

            QList<PrivacyListItem> privacyListItems = getPrivacyList(Utils::getBareJid(from),
                                                                                    privacyListName);
            return generatePrivacyListResult(from, id, privacyListName, privacyListItems);
        }
    }
    else if (iq.attribute("type") == "set")
    {
        QString firstChildTagName = query.firstChild().toElement().tagName();
        QString privacyListName = query.firstChild().toElement().attribute("name");
        if (((firstChildTagName == "default") || (firstChildTagName == "active")) &&
                !privacyListExist(Utils::getBareJid(from), privacyListName))
        {
            return Error::generateError("", "iq", "cancel", "item-not-found", "", from, id,
                                               iq.firstChildElement());
        }

        if (firstChildTagName == "active")
        {
            setActivePrivacyList(Utils::getBareJid(from), privacyListName);
            return generateIQResult(from, id);
        }
        else if (firstChildTagName == "default")
        {
            if (getDefaultPrivacyList(Utils::getBareJid(from)).isEmpty())
            {
                setDefaultPrivacyList(Utils::getBareJid(from), privacyListName);
                //m_userDefaultList.insert(Utils::getBareJid(from), privacyListName);
                return generateIQResult(from, id);
            }
            // default list is in use by another resource
            else
            {
                return Error::generateError("", "iq", "cancel", "conflict", "", from, id, query);
            }
        }
        else if (firstChildTagName == "list")
        {
            QDomNodeList itemList = query.firstChild().toElement().elementsByTagName("item");

            // Adding or updating item in privacy list
            if (!itemList.isEmpty())
            {
                QList<PrivacyListItem> privacyListItems;
                for (int i = 0, c = itemList.count(); i < c; ++i)
                {
                    QDomElement itemElement = itemList.item(i).toElement();
                    PrivacyListItem privacyListItem(itemElement.attribute("type"), itemElement.attribute("value"),
                                                    itemElement.attribute("action"), itemElement.attribute("order").toInt(),
                                                    QSet<QString>());

                    QDomNodeList itemChildList = itemElement.childNodes();
                    QSet<QString> itemChildName;
                    for (int j = 0, c = itemChildList.count(); j < c; ++j)
                    {
                        itemChildName << itemChildList.item(j).toElement().tagName();
                    }
                    privacyListItem.setItemChildElements(itemChildName);
                    privacyListItems << privacyListItem;
                }
                if (addItemsToPrivacyList(Utils::getBareJid(from), privacyListName, privacyListItems))
                {
                    emit sigPrivacyListPush(Utils::getBareJid(from), generatePrivacyPush(from, privacyListName, id));
                    return generateIQResult(from, id);
                }
                else
                {
                    return Error::generateInternalServerError();
                }
            }
            else
            {
                deletePrivacyList(Utils::getBareJid(from), privacyListName);
                return generateIQResult(from, id);
            }
        }
    }
    return QByteArray();
}

QByteArray PrivacyListManager::generatePrivacyListResult(QString to, QString id, QString privacyListName,
                                                         QList<PrivacyListItem> items)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:privacy");

    QDomElement listChild = document.createElement("list");
    listChild.setAttribute("name", privacyListName);

    for (int i = 0, c = items.count(); i < c; ++i)
    {
        listChild.appendChild(items.value(i).toXmlElement());
    }
    query.appendChild(listChild);
    iq.appendChild(query);
    document.appendChild(iq);

    // Request Acknowledgment of receipt
    // sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

QByteArray PrivacyListManager::privacyListNames(QString from, QString to, QString id, QStringList privacyListNames,
                                                QString activeListName, QString defaultListName)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");
    iq.setAttribute("from", from);
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:privacy");

    if (!activeListName.isEmpty())
    {
        QDomElement active = document.createElement("active");
        active.setAttribute("name", activeListName);
        query.appendChild(active);
    }

    if (!defaultListName.isEmpty())
    {
        QDomElement defaul = document.createElement("default");
        defaul.setAttribute("name", defaultListName);
        query.appendChild(defaul);
    }

    if (!privacyListNames.isEmpty())
    {
        foreach (QString privacyListName, privacyListNames)
        {
            QDomElement list = document.createElement("list");
            list.setAttribute("name", privacyListName);
            query.appendChild(list);
        }
    }

    iq.appendChild(query);
    document.appendChild(iq);

    // Request Acknowledgment of receipt
    // sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

QByteArray PrivacyListManager::generateIQResult(QString to, QString id)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    if (!to.isEmpty())
    {
        // Request Acknowledgment of receipt
        // sigSendReceiptRequest(to, document.toByteArray());
        iq.setAttribute("to", to);
    }
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    document.appendChild(iq);

    return document.toByteArray();
}

QDomDocument PrivacyListManager::generatePrivacyPush(QString to, QString privacyListName, QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "set");
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:privacy");

    QDomElement listElement = document.createElement("list");
    listElement.setAttribute("name", privacyListName);

    query.appendChild(listElement);
    iq.appendChild(query);
    document.appendChild(iq);

    // Request Acknowledgment of receipt
    // sigSendReceiptRequest(to, document.toByteArray());
    return document;
}

QByteArray PrivacyListManager::isBlocked(QString from, QString to, QString stanzaType)
{
    // PrivacyList <message/> block
    QString activeListName = getActivePrivacyList(Utils::getBareJid(to));
    QString defaultListName = getDefaultPrivacyList(Utils::getBareJid(to));

    QList<PrivacyListItem> privacyListDenyItems;
    QList<PrivacyListItem> privacyListAllowItems;
    if (!activeListName.isEmpty())
    {
        privacyListDenyItems = getPrivacyListItems(Utils::getBareJid(to), activeListName, stanzaType, "deny");
        privacyListAllowItems = getPrivacyListItems(Utils::getBareJid(to), activeListName, stanzaType, "allow");
    }
    else if (!defaultListName.isEmpty())
    {
        privacyListDenyItems = getPrivacyListItems(Utils::getBareJid(to), defaultListName, stanzaType, "deny");
        privacyListAllowItems = getPrivacyListItems(Utils::getBareJid(to), defaultListName, stanzaType, "allow");
    }
    else
        return QByteArray();

    // Test the allow privacy list items
    foreach (PrivacyListItem item, privacyListAllowItems)
    {
        if (item.getType().isEmpty())
        {
            return QByteArray();
        }
        else if (item.getType() == "jid")
        {
            if ((item.getValue() == from) ||
                    (item.getValue() == Utils::getBareJid(from)) ||
                    (item.getValue() == Utils::getHost(from)) ||
                    (item.getValue() == from.split("@").value(1)))
            {
                return QByteArray();
            }
        }
        else if (item.getType() == "group")
        {
            QList<QString> contactGroups = m_rosterMananager->getContactGroups(Utils::getBareJid(to), Utils::getBareJid(from)).toList();
            if (contactGroups.contains(item.getValue()))
            {
                return QByteArray();
            }
        }
        else if (item.getType() == "subscription")
        {
            QString contactSubscription = m_rosterMananager->getContactSubscription(Utils::getBareJid(to), Utils::getBareJid(from));
            if (contactSubscription == item.getValue())
            {
                return QByteArray();
            }
        }
    }

    // Test the deny privacy list items
    foreach (PrivacyListItem item, privacyListDenyItems)
    {
        if (item.getType().isEmpty())
        {
            if ((stanzaType != "presence-in") || (stanzaType != "presenceOut"))
            {
                return Error::generateError("", "iq", "cancel", "service-unavalaible",
                                             Utils::getHost(from), from, "", QDomElement());
            }
            else if ((stanzaType == "presence-in") || (stanzaType == "presenceOut"))
            {
                return QByteArray("a");
            }
        }
        else if (item.getType() == "jid")
        {
            if ((item.getValue() == from) ||
                    (item.getValue() == Utils::getBareJid(from)) ||
                    (item.getValue() == Utils::getHost(from)) ||
                    (item.getValue() == from.split("@").value(1)))
            {
                if ((stanzaType != "presence-in") || (stanzaType != "presenceOut"))
                {
                    return Error::generateError("", "iq", "cancel", "service-unavalaible",
                                                 Utils::getHost(from), from, "", QDomElement());
                }
                else if ((stanzaType == "presence-in") || (stanzaType == "presenceOut"))
                {
                    return QByteArray("a");
                }
            }
        }
        else if (item.getType() == "group")
        {
            QList<QString> contactGroups = m_rosterMananager->getContactGroups(Utils::getBareJid(to), Utils::getBareJid(from)).toList();
            if (contactGroups.contains(item.getValue()))
            {
                if ((stanzaType != "presence-in") || (stanzaType != "presenceOut"))
                {
                    return Error::generateError("", "iq", "cancel", "service-unavalaible",
                                                 Utils::getHost(from), from, "", QDomElement());
                }
                else if ((stanzaType == "presence-in") || (stanzaType == "presenceOut"))
                {
                    return QByteArray("a");
                }
            }
        }
        else if (item.getType() == "subscription")
        {
            QString contactSubscription = m_rosterMananager->getContactSubscription(Utils::getBareJid(to), Utils::getBareJid(from));
            if (contactSubscription == item.getValue())
            {
                if ((stanzaType != "presence-in") || (stanzaType != "presenceOut"))
                {
                    return Error::generateError("", "iq", "cancel", "service-unavalaible",
                                                 Utils::getHost(from), from, "", QDomElement());
                }
                else if ((stanzaType == "presence-in") || (stanzaType == "presenceOut"))
                {
                    return QByteArray("a");
                }
            }
        }
    }
    return QByteArray();
}

QByteArray PrivacyListManager::isBlocked(QString from, QString to,
                                         QList<PrivacyListItem> privacyListAllowItems,
                                         QList<PrivacyListItem> privacyListDenyItems)
{
    // Test the allow privacy list items
    foreach (PrivacyListItem item, privacyListAllowItems)
    {
        if (item.getType().isEmpty())
        {
            return QByteArray();
        }
        else if (item.getType() == "jid")
        {
            if ((item.getValue() == from) ||
                    (item.getValue() == Utils::getBareJid(from)) ||
                    (item.getValue() == Utils::getHost(from)) ||
                    (item.getValue() == from.split("@").value(1)))
            {
                return QByteArray();
            }
        }
        else if (item.getType() == "group")
        {
            QList<QString> contactGroups = m_rosterMananager->getContactGroups(Utils::getBareJid(to), Utils::getBareJid(from)).toList();
            if (contactGroups.contains(item.getValue()))
            {
                return QByteArray();
            }
        }
        else if (item.getType() == "subscription")
        {
            QString contactSubscription = m_rosterMananager->getContactSubscription(Utils::getBareJid(to), Utils::getBareJid(from));
            if (contactSubscription == item.getValue())
            {
                return QByteArray();
            }
        }
    }

    // Test the deny privacy list items
    foreach (PrivacyListItem item, privacyListDenyItems)
    {
        if (item.getType().isEmpty())
        {
            return QByteArray("a");
        }
        else if (item.getType() == "jid")
        {
            if ((item.getValue() == from) ||
                    (item.getValue() == Utils::getBareJid(from)) ||
                    (item.getValue() == Utils::getHost(from)) ||
                    (item.getValue() == from.split("@").value(1)))
            {
                return QByteArray("a");
            }
        }
        else if (item.getType() == "group")
        {
            QList<QString> contactGroups = m_rosterMananager->getContactGroups(Utils::getBareJid(to), Utils::getBareJid(from)).toList();
            if (contactGroups.contains(item.getValue()))
            {
                return QByteArray("a");
            }
        }
        else if (item.getType() == "subscription")
        {
            QString contactSubscription = m_rosterMananager->getContactSubscription(Utils::getBareJid(to), Utils::getBareJid(from));
            if (contactSubscription == item.getValue())
            {
                return QByteArray("a");
            }
        }
    }
    return QByteArray();
}

QList<PrivacyListItem> PrivacyListManager::getPrivacyList(QString jid, QString privacyListName)
{
    return m_storageManager->getStorage()->getPrivacyList(jid, privacyListName);
}

QStringList PrivacyListManager::getPrivacyListNames(QString jid)
{
    return m_storageManager->getStorage()->getPrivacyListNames(jid);
}

QString PrivacyListManager::getDefaultPrivacyList(QString jid)
{
    return m_storageManager->getStorage()->getDefaultPrivacyList(jid);
}

QString PrivacyListManager::getActivePrivacyList(QString jid)
{
    return m_storageManager->getStorage()->getActivePrivacyList(jid);
}

bool PrivacyListManager::setDefaultPrivacyList(QString jid, QString defaultList)
{
    return m_storageManager->getStorage()->setDefaultPrivacyList(jid, defaultList);
}

bool PrivacyListManager::setActivePrivacyList(QString jid, QString activeList)
{
    return m_storageManager->getStorage()->setActivePrivacyList(jid, activeList);
}

QList<PrivacyListItem> PrivacyListManager::getPrivacyListItems(QString jid, QString privacyListName,
                                                                   QString stanzaType, QString action)
{
    return m_storageManager->getStorage()->getPrivacyListItems(jid, privacyListName, stanzaType, action);
}

bool PrivacyListManager::addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items)
{
    return m_storageManager->getStorage()->addItemsToPrivacyList(jid, privacyListName, items);
}

bool PrivacyListManager::deletePrivacyList(QString jid, QString privacyListName)
{
    return m_storageManager->getStorage()->deletePrivacyList(jid, privacyListName);
}

bool PrivacyListManager::privacyListExist(QString jid, QString privacyListName)
{
    return m_storageManager->getStorage()->privacyListExist(jid, privacyListName);
}
