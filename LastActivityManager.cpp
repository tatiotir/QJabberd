#include "LastActivityManager.h"

/*!
 * \brief LastActivityManager::LastActivityManager constructor of the last activity manager
 * \param userManager
 * \param rosterManager
 * \param storageManager
 */
LastActivityManager::LastActivityManager(UserManager *userManager, RosterManager *rosterManager,
                                         StorageManager *storageManager)
{
    m_userManager = userManager;
    m_rosterManager = rosterManager;
    m_storageManager = storageManager;
}

/*!
 * \brief The LastActivityManager::lastActivityReply method eturn response from last activity query
 * \param iqXML
 * \param iqFrom
 * \return QByteArray
 */
QByteArray LastActivityManager::lastActivityReply(QDomDocument document, QString iqFrom)
{
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
                    int seconds = qAbs(QDateTime::fromString(lastLogoutTime).secsTo(QDateTime::currentDateTime()));
                    QString lastStatus = getLastStatus(to);

                    return lastActivityQueryResult(to, from, id, seconds, lastStatus);
                }
                else
                {
                    return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());
                }
            }
            else
            {
                return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());
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
            return Error::generateError("", "iq", "auth", "forbidden", to, from, id, QDomElement());
        }
    }
}

/*!
 * \brief The LastActivityManager::lastActivityQueryResult method return last activity result from iq request
 * \param from
 * \param to
 * \param id
 * \param seconds
 * \param lastStatus
 * \return QByteArray
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

/*!
 * \brief The LastActivityManager::getLastLogoutTime method get the last user logout time
 * \param jid
 * \return QString
 */
QString LastActivityManager::getLastLogoutTime(QString jid)
{
    return m_storageManager->getStorage()->getLastLogoutTime(jid);
}

/*!
 * \brief The LastActivityManager::setLastLogoutTime method set the last user logout time
 * \param jid
 * \param lastLogoutTime
 */
void LastActivityManager::setLastLogoutTime(QString jid, QString lastLogoutTime)
{
    m_storageManager->getStorage()->setLastLogoutTime(jid, lastLogoutTime);
}

/*!
 * \brief The LastActivityManager::getLastStatus method get the last user status
 * \param jid
 * \return QString
 */
QString LastActivityManager::getLastStatus(QString jid)
{
    return m_storageManager->getStorage()->getLastStatus(jid);
}

/*!
 * \brief The LastActivityManager::setLastStatus method set the last user status
 * \param jid
 * \param status
 */
void LastActivityManager::setLastStatus(QString jid, QString status)
{
    m_storageManager->getStorage()->setLastStatus(jid, status);
}
