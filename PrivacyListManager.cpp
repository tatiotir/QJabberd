#include "PrivacyListManager.h"

PrivacyListManager::PrivacyListManager(StorageManager *storageManager)
{
    m_storageManager =  storageManager;
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
        qDebug() << "query text : " << query.text();
        if (query.firstChild().toElement().isNull())
        {
            return privacyListNames(to, from, id);
        }
        else
        {
            QString privacyListName = query.firstChild().toElement().attribute("name");
            if ((privacyListName != "private") || (privacyListName != "public") || (privacyListName != "special"))
            {
                return Error::generateError("iq", "cancel", "item-not-found", from, "", id,
                                                   iq.firstChildElement());
            }

            if (query.elementsByTagName("list").count() > 1)
            {
                return Error::generateError("iq", "modify", "bad-request", "", from, id, iq.firstChildElement());
            }

            QList<PrivacyListItem> privacyListItems = getPrivacyList(Utils::getBareJid(from),
                                                                                    privacyListName);
            return generatePrivacyListResult(from, id, privacyListName, privacyListItems);
        }
    }
    else if (query.attribute("type") == "set")
    {
        QString firstChildTagName = query.firstChild().toElement().tagName();
        QString firstChildAttributeName = query.firstChild().toElement().attribute("name", "");
        if ((!firstChildAttributeName.isEmpty()) && ((firstChildAttributeName != "private") ||
                                                     (firstChildAttributeName != "public") ||
                                                     (firstChildAttributeName != "special")))
        {
            return Error::generateError("iq", "cancel", "item-not-found", from, "", id,
                                               iq.firstChildElement());
        }

        if (firstChildTagName == "active")
        {
            m_privacyActiveListName = firstChildAttributeName;
            return generateIQResult(from, id);
        }
        else if (firstChildTagName == "default")
        {
            emit sigSetDefaultListName(Utils::getBareJid(from), from, firstChildAttributeName, id);
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
                if (addItemsToPrivacyList(Utils::getBareJid(from), firstChildAttributeName, privacyListItems))
                {
                    emit sigPrivacyListPush(Utils::getBareJid(from), generatePrivacyPush(from, firstChildAttributeName, id));
                    return generateIQResult(from, id);
                }
                else
                {
                    // Return internal server error
                }
            }
            else
            {
                deletePrivacyList(Utils::getBareJid(from), firstChildAttributeName);
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
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

QByteArray PrivacyListManager::privacyListNames(QString from, QString to, QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");
    iq.setAttribute("from", from);
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:privacy");

    QDomElement active = document.createElement("active");
    active.setAttribute("name", "private");

    QDomElement defaul = document.createElement("default");
    defaul.setAttribute("name", "public");

    QDomElement list1 = document.createElement("list");
    list1.setAttribute("name", "public");

    QDomElement list2 = document.createElement("list");
    list2.setAttribute("name", "private");

    QDomElement list3 = document.createElement("list");
    list3.setAttribute("name", "special");

    query.appendChild(active);
    query.appendChild(defaul);
    query.appendChild(list1);
    query.appendChild(list2);
    query.appendChild(list3);

    iq.appendChild(query);
    document.appendChild(iq);

    // Request Acknowledgment of receipt
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

QByteArray PrivacyListManager::generateIQResult(QString to, QString id)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    if (!to.isEmpty())
    {
        // Request Acknowledgment of receipt
        sigSendReceiptRequest(to, document.toByteArray());
        iq.setAttribute("to", to);
    }
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    document.appendChild(iq);

    return document.toByteArray();
}

QByteArray PrivacyListManager::generatePrivacyPush(QString to, QString privacyListName, QString id)
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
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

void PrivacyListManager::setPrivacyActiveList(QString activeList)
{
    m_privacyActiveListName = activeList;
}

QList<PrivacyListItem> PrivacyListManager::getPrivacyList(QString jid, QString privacyListName)
{
    return m_storageManager->getStorage()->getPrivacyList(jid, privacyListName);
}

bool PrivacyListManager::addItemsToPrivacyList(QString jid, QString privacyListName, QList<PrivacyListItem> items)
{
    return m_storageManager->getStorage()->addItemsToPrivacyList(jid, privacyListName, items);
}

bool PrivacyListManager::deletePrivacyList(QString jid, QString privacyListName)
{
    return m_storageManager->getStorage()->deletePrivacyList(jid, privacyListName);
}

QString PrivacyListManager::getPrivacyActiveList()
{
    return m_privacyActiveListName;
}

void PrivacyListManager::setPrivacyDefaultList(QString activeList)
{
    m_privacyActiveListName = activeList;
}

QString PrivacyListManager::getPrivacyDefaultList()
{
    return m_privacyActiveListName;
}
