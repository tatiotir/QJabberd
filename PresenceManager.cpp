#include "PresenceManager.h"

/*!
 * \brief PresenceManager::PresenceManager PresenceManager constructor
 * \param usermanager
 * \param rosterManager
 * \param lastActivityManager
 * \param privateListManager
 */
PresenceManager::PresenceManager(UserManager *usermanager, RosterManager *rosterManager,
                                 LastActivityManager *lastActivityManager,
                                 PrivacyListManager *privateListManager)
{
    m_userManager = usermanager;
    m_rosterManager = rosterManager;
    m_lastActivityManager = lastActivityManager;
    m_privacyListManager = privateListManager;
    m_clientSendFirstPresence = false;
}

/*!
 * \brief The PresenceManager::parsePresence method parse presence stanza and output a response in a byte array
 * \param presenceXML
 * \param presenceFrom
 * \return QByteArray
 */
QByteArray PresenceManager::parsePresence(QDomDocument document, QString presenceFrom)
{
    QDomElement presence = document.documentElement();
    QString presenceType = presence.attribute("type", "");
    QString from = Utils::getBareJid(presence.attribute("from", presenceFrom));
    QString to = Utils::getBareJid(presence.attribute("to"));

    if (presenceType == "subscribe")
    {
        if ((m_rosterManager->contactExists(from, to)))
        {
            Contact userContact = m_rosterManager->getContact(from, to);
            cout << userContact;
            if ((userContact.getSubscription() != "both") && (userContact.getSubscription() != "to"))
            {
                // Stamp the outbound subscription
                presence.setAttribute("from", from);

                // Locally deliver the presence stanza
                emit sigPresenceBroadCast(to, document);

                m_rosterManager->updateAskAttributeToContact(from, to, "subscribe");

                userContact = m_rosterManager->getContact(from, to);

                // send roster push to the user's interested resources
                emit sigRosterPush(from, Utils::generateRosterPush(from, Utils::generateId(),
                                                                   to,
                                                                   userContact.getName(),
                                                                   userContact.getSubscription(),
                                                                   userContact.getAsk(),
                                                                   userContact.getApproved(),
                                                                   userContact.getGroups()));
            }
        }
    }
    else if (presenceType == "unsubscribe")
    {
        if (m_rosterManager->contactExists(from, to))
        {
            Contact userContact = m_rosterManager->getContact(from, to);
            if (userContact.getSubscription() == "to")
                m_rosterManager->updateSubscriptionToContact(from, to, "none");

            if (userContact.getSubscription() == "both")
                m_rosterManager->updateSubscriptionToContact(from, to, "from");

            m_rosterManager->updateAskAttributeToContact(from, to, "");

            // Stamp the outbound subscription
            presence.setAttribute("from", from);

            // Locally deliver the presence stanza
            emit sigPresenceBroadCast(to, document);

            // Refresh user contact
            userContact = m_rosterManager->getContact(from, to);

            // Roster push
            emit sigRosterPush(from,  Utils::generateRosterPush(from, Utils::generateId(),
                                                             to,
                                                             userContact.getName(),
                                                             userContact.getSubscription(),
                                                             userContact.getAsk(),
                                                             userContact.getApproved(),
                                                             userContact.getGroups()));

        }
    }
    else if (presenceType == "subscribed")
    {
        if (m_rosterManager->contactExists(from, to))
        {
            Contact userContact = m_rosterManager->getContact(from, to);
            Contact contactUser = m_rosterManager->getContact(to, from);

            if (((userContact.getSubscription() == "to") && (contactUser.getAsk() == "subscribe"))
                    || ((userContact.getSubscription() == "none") && (contactUser.getAsk() == "subscribe"))
                    || ((userContact.getSubscription() == "none") && (userContact.getAsk() == "subscribe") &&
                        (contactUser.getAsk() == "subscribe")))
            {
                qDebug() << "user contact subscription : " << userContact.getSubscription();
                qDebug() << "user contact approved : " << userContact.getApproved();
                qDebug() << "contact user : " << contactUser.getAsk();

                // Stamp the outbound subscription
                presence.setAttribute("from", from);

                // Locally deliver the presence stanza
                emit sigPresenceBroadCast(to, document);

                m_rosterManager->updateAskAttributeToContact(from, to, "");
                m_rosterManager->updateSubscriptionToContact(from, to, "from");
                userContact = m_rosterManager->getContact(from, to);

                // send roster push to the contact's interested resources
                emit sigRosterPush(from, Utils::generateRosterPush(from, Utils::generateId(), to,
                                                                   userContact.getName(),
                                                                   userContact.getSubscription(),
                                                                   userContact.getAsk(),
                                                                   userContact.getApproved(),
                                                                   userContact.getGroups()));

                // Send presence to the user to which the user subscribed to
                emit sigPresenceBroadCastFromContact(to, from);
                deleteOfflinePresenceSubscribe(to, from);
            }
            else if ((userContact.getSubscription() == "to") || (userContact.getSubscription() == "none")
                     || ((userContact.getSubscription() == "none") && userContact.getAsk() == "subscribe"))
            {
                // Update the approved (subscription pre-approval)
                m_rosterManager->updateApprovedToContact(from, to, true);
                userContact = m_rosterManager->getContact(from, to);

                // send roster push to the contact's interested resources
                emit sigRosterPush(from, Utils::generateRosterPush(from, Utils::generateId(), to,
                                                                   userContact.getName(),
                                                                   userContact.getSubscription(),
                                                                   userContact.getAsk(),
                                                                   true,
                                                                   userContact.getGroups()));
            }
        }
        else
        {
            Contact userContact("", true, "", to, "", "none", QSet<QString>());
            m_rosterManager->addContactToRoster(from, userContact);

            // send roster push to the contact's interested resources
            emit sigRosterPush(from, Utils::generateRosterPush(from, Utils::generateId(), to,
                                                               userContact.getName(),
                                                               userContact.getSubscription(),
                                                               userContact.getAsk(),
                                                               userContact.getApproved(),
                                                               userContact.getGroups()));
        }
    }
    else if (presenceType == "unsubscribed")
    {
        if (m_rosterManager->contactExists(from, to))
        {
            Contact userContact = m_rosterManager->getContact(from, to);
            if (((userContact.getSubscription() == "none")
                    || ((userContact.getSubscription() == "none") && userContact.getAsk() == "subscribe")
                    || (userContact.getSubscription() == "to")) && userContact.getApproved())
            {
                // Remove the pre-approval to this contact
                m_rosterManager->updateApprovedToContact(from, to, false);
            }
            else
            {
                // Stamp the outbound subscription
                presence.setAttribute("from", from);

                // Locally deliver the presence stanza
                emit sigPresenceBroadCast(to, document);

                // Send presence of type unavailable to the user
                emit sigPresenceUnavailableBroadCast(to, from);

                Contact userContact = m_rosterManager->getContact(from, to);

                if (userContact.getSubscription() == "from")
                    m_rosterManager->updateSubscriptionToContact(from, to, "none");

                if (userContact.getSubscription() == "both")
                    m_rosterManager->updateSubscriptionToContact(from, to, "to");

                // Refresh user contact
                userContact = m_rosterManager->getContact(from, to);

                // send roster push to the user's interested resources
                emit sigRosterPush(from, Utils::generateRosterPush(from, Utils::generateId(), to,
                                                                   userContact.getName(),
                                                                   userContact.getSubscription(),
                                                                   userContact.getAsk(),
                                                                   userContact.getApproved(),
                                                                   userContact.getGroups()));

                deleteOfflinePresenceSubscribe(to, from);
            }
        }
    }
    else if (presenceType.isEmpty() && to.isEmpty())
    {
        QString priority = presence.elementsByTagName("priority").item(0).toElement().text();
        emit sigPresencePriority(presenceFrom, priority.toInt());

        QList<Contact> contactList = m_rosterManager->getContactsList(from);
        for (int i = 0; i < contactList.count(); ++i)
        {
            Contact contact = contactList.value(i);
            if ((contact.getSubscription() == "from") || (contact.getSubscription() == "both"))
            {
                document.documentElement().setAttribute("from", presenceFrom);
                document.documentElement().setAttribute("to", contact.getJid());
                emit sigPresenceBroadCast(contact.getJid(), document);
            }
            if ((contact.getSubscription() == "to") || (contact.getSubscription() == "both"))
            {
                emit sigPresenceProbeToContact(contact.getJid(), presenceFrom, false);
            }
        }

        emit sigCurrentPresence(presenceFrom, document.toByteArray());

        document.documentElement().setAttribute("from", presenceFrom);
        document.documentElement().setAttribute("to", Utils::getBareJid(from));
        emit sigPresenceBroadCast(Utils::getBareJid(from), document);
    }
    else if ((presenceType == "unavailable") && to.isEmpty())
    {
        emit sigCurrentPresence(presenceFrom, document.toByteArray());

        QList<Contact> contactList = m_rosterManager->getContactsList(from);
        for (int i = 0; i < contactList.count(); ++i)
        {
            Contact contact = contactList.value(i);
            if ((contact.getSubscription() == "from") || (contact.getSubscription() == "both"))
            {
                document.documentElement().setAttribute("from", presenceFrom);
                document.documentElement().setAttribute("to", contact.getJid());
                emit sigPresenceBroadCast(contact.getJid(), document);
            }
        }
        m_lastActivityManager->setLastStatus(from, presence.elementsByTagName("status").item(0).toElement().text());
        m_clientSendFirstPresence = false;
    }
    else if (presenceType == "probe")
    {
        // Presence probe send in the case of directed presence
        emit sigPresenceProbeToContact(to, from, true);
    }
    else // Directed presence
    {
        emit sigDirectedPresence(from, to, document.toByteArray());
    }

    return QByteArray();
}

void PresenceManager::presenceBroadcast(QString to, QDomDocument document)
{
    emit sigPresenceBroadCast(to, document);
}

void PresenceManager::presenceUnavailableBroadcast(QString to, QString from)
{
    emit sigPresenceUnavailableBroadCast(to, from);
}

/*!
 * \brief The PresenceManager::deleteOfflinePresenceSubscribe method delete offline presence subscription of type "subscribe" using the key (from, to)
 * \param from
 * \param to
 */
void PresenceManager::deleteOfflinePresenceSubscribe(QString from, QString to)
{
    m_userManager->getStorageManager()->getStorage()->deleteOfflinePresenceSubscribe(from, to);
}

QDomDocument PresenceManager::generatePresence(QString type, QString from, QString to, QString id, QString show,
                                   QString priority, QMultiHash<QString, QString> status)
{
    QDomDocument document;

    QDomElement presence = document.createElement("presence");

    if (!type.isEmpty())
        presence.setAttribute("type", type);

    if (!from.isEmpty())
        presence.setAttribute("from", from);

    if (!to.isEmpty())
        presence.setAttribute("to", to);

    if (!id.isEmpty())
        presence.setAttribute("id", id);

    if (!show.isEmpty())
    {
        QDomElement showNode = document.createElement("show");
        showNode.appendChild(document.createTextNode(show));
        presence.appendChild(showNode);
    }

    if (!priority.isEmpty())
    {
        QDomElement priorityNode = document.createElement("priority");
        priorityNode.appendChild(document.createTextNode(priority));
        presence.appendChild(priorityNode);
    }

    if (!status.isEmpty())
    {
        QList<QString> keys = status.keys();
        for (int i = 0, c = keys.count(); i < c; ++i)
        {
            QDomElement statusNode = document.createElement("status");
            statusNode.setAttribute("xml:lang", status.value(keys.value(i)));
            statusNode.appendChild(document.createTextNode(status.value(keys.value(i))));
            presence.appendChild(statusNode);
        }
    }

    document.appendChild(presence);
    return document;
}

QDomDocument PresenceManager::generatePresence(QString type, QString from, QString to, QString id, QString show,
                                   QString priority, QString status)
{
    QDomDocument document;

    QDomElement presence = document.createElement("presence");

    if (!type.isEmpty())
        presence.setAttribute("type", type);

    if (!from.isEmpty())
        presence.setAttribute("from", from);

    if (!to.isEmpty())
        presence.setAttribute("to", to);

    if (!id.isEmpty())
        presence.setAttribute("id", id);

    if (!show.isEmpty())
    {
        QDomElement showNode = document.createElement("show");
        showNode.appendChild(document.createTextNode(show));
        presence.appendChild(showNode);
    }

    if (!priority.isEmpty())
    {
        QDomElement priorityNode = document.createElement("priority");
        priorityNode.appendChild(document.createTextNode(priority));
        presence.appendChild(priorityNode);
    }

    if (!status.isEmpty())
    {
        QDomElement statusNode = document.createElement("status");
        statusNode.appendChild(document.createTextNode(status));
        presence.appendChild(statusNode);
    }

    document.appendChild(presence);
    return document;
}
