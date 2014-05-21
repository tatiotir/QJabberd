#include "LastActivityManager.h"

/**
 * Constructor of the last activity manager
 *
 * @brief LastActivityManager::LastActivityManager
 * @param userManager
 * @param rosterManager
 * @param storageManager
 */
LastActivityManager::LastActivityManager(UserManager *userManager, RosterManager *rosterManager,
                                         StorageManager *storageManager)
{
    m_userManager = userManager;
    m_rosterManager = rosterManager;
    m_storageManager = storageManager;
}

/**
 * Return reply from an last activity query
 *
 * @brief LastActivityManager::lastActivityReply
 * @param iqXML
 * @param iqFrom
 * @return QByteArray
 */
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

/**
 * Return last activity result from iq request
 *
 * @brief LastActivityManager::lastActivityQueryResult
 * @param from
 * @param to
 * @param id
 * @param seconds
 * @param lastStatus
 * @return QByteArray
 */
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

/**
 * Get the last logout for a user
 *
 * @brief LastActivityManager::getLastLogoutTime
 * @param jid
 * @return QString
 */
QString LastActivityManager::getLastLogoutTime(QString jid)
{
    return m_storageManager->getStorage()->getLastLogoutTime(jid);
}

/**
 * Set the last logout time from a user
 *
 *  @brief LastActivityManager::setLastLogoutTime
 * @param jid
 * @param lastLogoutTime
 */
void LastActivityManager::setLastLogoutTime(QString jid, QString lastLogoutTime)
{
    m_storageManager->getStorage()->setLastLogoutTime(jid, lastLogoutTime);
}

/**
 * Get the last status for an user
 *
 * @brief LastActivityManager::getLastStatus
 * @param jid
 * @return QString
 */
QString LastActivityManager::getLastStatus(QString jid)
{
    return m_storageManager->getStorage()->getLastStatus(jid);
}

/**
 * Set the last status for an user
 *
 * @brief LastActivityManager::setLastStatus
 * @param jid
 * @param status
 */
void LastActivityManager::setLastStatus(QString jid, QString status)
{
    m_storageManager->getStorage()->setLastStatus(jid, status);
}
