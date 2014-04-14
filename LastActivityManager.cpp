#include "LastActivityManager.h"

LastActivityManager::LastActivityManager(UserManager *userManager, RosterManager *rosterManager,
                                         StorageManager *storageManager)
{
    m_userManager = userManager;
    m_rosterManager = rosterManager;
    m_storageManager = storageManager;
}

QByteArray LastActivityManager::lastActivityReply(QByteArray iqXML, QString iqFrom)
{
    QDomDocument document;
    document.setContent(iqXML);

    QString from = document.documentElement().attribute("from", iqFrom);
    QString to = document.documentElement().attribute("to");
    QString id = document.documentElement().attribute("id", Utils::generateId());

    if (!to.contains("/"))
    {
        // last activity query to server host
        if (to == "localhost")
        {
            emit sigServerLastActivityQuery(from, to, id);
            return QByteArray();
        }
        else
        {
            if (m_rosterManager->contactExists(to, Utils::getBareJid(from)))
            {
                Contact contact = m_rosterManager->getContact(to, Utils::getBareJid(from));
                if ((!contact.getSubscription().isEmpty() && (contact.getSubscription() != "none")))
                {
                    QString lastLogoutTime = getLastLogoutTime(to);
                    int seconds = qAbs(QTime::fromString(lastLogoutTime).secsTo(QTime::currentTime()));
                    QString lastStatus = getLastStatus(to);

                    return lastActivityQueryResult(to, from, id, seconds, lastStatus);
                }
                else
                {
                    return Error::generateError("iq", "auth", "forbidden", to, from, id, QDomElement());
                }
            }
            else
            {
                return Error::generateError("iq", "auth", "forbidden", to, from, id, QDomElement());
            }
        }
    }
    else
    {
        if ((m_rosterManager->contactExists(Utils::getBareJid(to), Utils::getBareJid(from))))
        {
            Contact contact = m_rosterManager->getContact(Utils::getBareJid(to), Utils::getBareJid(from));
            if ((!contact.getSubscription().isEmpty() && (contact.getSubscription() != "none")))
            {
                emit sigLastActivityQuery(from, to, id, getLastStatus(Utils::getBareJid(to)));
            }
            return QByteArray();
        }
        else
        {
            qDebug() << "last activity laba";
            return Error::generateError("iq", "auth", "forbidden", to, from, id, QDomElement());
        }
    }
}

QByteArray LastActivityManager::lastActivityQueryResult(QString from, QString to, QString id, int seconds, QString lastStatus)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("from", to);
    iq.setAttribute("to", from);
    iq.setAttribute("type", "result");
    iq.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:last");
    query.setAttribute("seconds", seconds);

    if (!lastStatus.isEmpty())
    {
        query.appendChild(document.createTextNode(lastStatus));
    }
    iq.appendChild(query);
    document.appendChild(iq);

    // Request Acknowledgment of receipt
    sigSendReceiptRequest(from, document.toByteArray());
    return document.toByteArray();
}

QString LastActivityManager::getLastLogoutTime(QString jid)
{
    return m_storageManager->getStorage()->getLastLogoutTime(jid);
}

void LastActivityManager::setLastLogoutTime(QString jid, QString lastLogoutTime)
{
    m_storageManager->getStorage()->setLastLogoutTime(jid, lastLogoutTime);
}

QString LastActivityManager::getLastStatus(QString jid)
{
    return m_storageManager->getStorage()->getLastStatus(jid);
}

void LastActivityManager::setLastStatus(QString jid, QString status)
{
    m_storageManager->getStorage()->setLastStatus(jid, status);
}
