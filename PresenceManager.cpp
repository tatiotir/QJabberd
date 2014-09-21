#include "PresenceManager.h"

/*!
 * \brief PresenceManager::PresenceManager PresenceManager constructor
 * \param usermanager
 * \param rosterManager
 * \param lastActivityManager
 * \param privateListManager
 */
PresenceManager::PresenceManager(QObject *parent, UserManager *usermanager, RosterManager *rosterManager,
                                 LastActivityManager *lastActivityManager,
                                 PrivacyListManager *privateListManager, MucManager *mucManager,
                                 BlockingCommandManager *blockingCmdManager) : QObject(parent)
{
    m_userManager = usermanager;
    m_rosterManager = rosterManager;
    m_lastActivityManager = lastActivityManager;
    m_privacyListManager = privateListManager;
    m_mucManager = mucManager;
    m_blockingCmdManager = blockingCmdManager;
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
    QString presenceTo = presence.attribute("to");
    QString id = presence.attribute("id", Utils::generateId());

//    if (((presenceType == "unavailable") && to.isEmpty()) || (presenceType.isEmpty() && to.isEmpty()))
//    {
//        QByteArray privacyListError = m_privacyListManager->isBlocked(presence.attribute("from"), presence.attribute("to"), "presence-in");
//        if (!privacyListError.isEmpty())
//            return privacyListError;
//        else if (privacyListError == "a")
//            return QByteArray();

//        QByteArray privacyListError1 = m_privacyListManager->isBlocked(presence.attribute("from"), presence.attribute("to"), "presence-out");
//        if (!privacyListError1.isEmpty())
//            return privacyListError1;
//        else if (privacyListError1 == "a")
//            return QByteArray();
//    }

    if (presenceType == "subscribe" || (presenceType == "unsubscribe") || (presenceType == "subscribed") ||
            (presenceType == "unsubscribed") || ((presenceType == "unavailable") && to.isEmpty()) ||
            ((presenceType == "unavailable") && to.isEmpty()) || (presenceType == "probe"))
    {
        QByteArray blockListError = m_blockingCmdManager->isBlocked(presence.attribute("from"), presence.attribute("to"),
                                                           "presence");
        if (!blockListError.isEmpty())
            return blockListError;
        else if (blockListError == "a")
            return QByteArray();
    }

    if (presenceType == "subscribe")
    {
        if (!m_rosterManager->contactExists(from, to))
            m_rosterManager->addContactToRoster(from, Contact("", false, "", to, Utils::getUsername(to), "none", QSet<QString>()));

//        if ((m_rosterManager->contactExists(from, to)))
//        {
            Contact userContact = m_rosterManager->getContact(from, to);
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
            //}
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
        QString activeListName = m_privacyListManager->getActivePrivacyList(from);
        QString defaultListName = m_privacyListManager->getDefaultPrivacyList(from);

        QList<PrivacyListItem> privacyListDenyItems;
        QList<PrivacyListItem> privacyListAllowItems;
        if (!activeListName.isEmpty())
        {
            privacyListDenyItems = m_privacyListManager->getPrivacyListItems(from, activeListName, "presence-out", "deny");
            privacyListAllowItems = m_privacyListManager->getPrivacyListItems(from, activeListName, "presence-out", "allow");
        }
        else if (!defaultListName.isEmpty())
        {
            privacyListDenyItems = m_privacyListManager->getPrivacyListItems(from, defaultListName, "presence-out", "deny");
            privacyListAllowItems = m_privacyListManager->getPrivacyListItems(from, defaultListName, "presence-out", "allow");
        }

        QString priority = presence.elementsByTagName("priority").item(0).toElement().text();
        emit sigPresencePriority(presenceFrom, priority.toInt());

        QList<Contact> contactList = m_rosterManager->getContactsList(from);
        for (int i = 0; i < contactList.count(); ++i)
        {
            Contact contact = contactList.value(i);
            if ((contact.getSubscription() == "from") || (contact.getSubscription() == "both"))
            {
                QByteArray privacyListError = m_privacyListManager->isBlocked(presenceFrom, contact.getJid(), "presence-in");
                if (privacyListError == "a")
                    return QByteArray();

                QByteArray privacyListError1 = m_privacyListManager->isBlocked(contact.getJid(), from, privacyListAllowItems, privacyListDenyItems);
                if (privacyListError1 == "a")
                    return QByteArray();

                document.documentElement().setAttribute("from", presenceFrom);
                document.documentElement().setAttribute("to", contact.getJid());
                emit sigPresenceBroadCast(contact.getJid(), document);
            }
            if ((contact.getSubscription() == "to") || (contact.getSubscription() == "both"))
            {
                QByteArray privacyListError = m_privacyListManager->isBlocked(contact.getJid(), from, privacyListAllowItems, privacyListDenyItems);
                if (privacyListError == "a")
                    return QByteArray();

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
        QString activeListName = m_privacyListManager->getActivePrivacyList(from);
        QString defaultListName = m_privacyListManager->getDefaultPrivacyList(from);

        QList<PrivacyListItem> privacyListDenyItems;
        QList<PrivacyListItem> privacyListAllowItems;
        if (!activeListName.isEmpty())
        {
            privacyListDenyItems = m_privacyListManager->getPrivacyListItems(from, activeListName, "presence-out", "deny");
            privacyListAllowItems = m_privacyListManager->getPrivacyListItems(from, activeListName, "presence-out", "allow");
        }
        else if (!defaultListName.isEmpty())
        {
            privacyListDenyItems = m_privacyListManager->getPrivacyListItems(from, defaultListName, "presence-out", "deny");
            privacyListAllowItems = m_privacyListManager->getPrivacyListItems(from, defaultListName, "presence-out", "allow");
        }

        emit sigCurrentPresence(presenceFrom, document.toByteArray());

        QList<Contact> contactList = m_rosterManager->getContactsList(from);
        for (int i = 0; i < contactList.count(); ++i)
        {
            Contact contact = contactList.value(i);
            if ((contact.getSubscription() == "from") || (contact.getSubscription() == "both"))
            {
                QByteArray privacyListError = m_privacyListManager->isBlocked(presenceFrom, contact.getJid(), "presence-in");
                if (privacyListError == "a")
                    return QByteArray();

                QByteArray privacyListError1 = m_privacyListManager->isBlocked(contact.getJid(), from, privacyListAllowItems, privacyListDenyItems);
                if (privacyListError1 == "a")
                    return QByteArray();

                document.documentElement().setAttribute("from", presenceFrom);
                document.documentElement().setAttribute("to", contact.getJid());
                emit sigPresenceBroadCast(contact.getJid(), document);
            }
        }
        m_lastActivityManager->setLastStatus(from, presence.elementsByTagName("status").item(0).toElement().text());
        m_clientSendFirstPresence = false;
    }
    else if ((presenceType == "unavailable") && !to.isEmpty())
    {
        QString roomName = to;
        emit sigMucPresenceBroadCast(from,
                                     Utils::generatePresence("unavailable", presenceTo, from, "", "member", "none",
                                                             from, "", QList<int>() << 110, "", ""));
        // remove the user on the occupants list
        m_mucManager->removeOccupant(roomName, presenceTo);

        QList<Occupant> occupantsList = m_mucManager->getOccupants(roomName);

        // There is no occupant any more and the room isn't persistent
        if (occupantsList.isEmpty() && !m_mucManager->isPersistentRoom(roomName))
        {
            m_mucManager->destroyRoom(roomName);
        }
        else
        {
            foreach (Occupant occupant, occupantsList)
            {
                emit sigMucPresenceBroadCast(occupant.jid(),
                                             Utils::generatePresence("unavailable", presenceTo, occupant.jid(), "",
                                                                     "member", "none", from, "", QList<int>(), "", ""));
            }
        }
    }
    else if (presenceType == "probe")
    {
        // Presence probe send in the case of directed presence
        emit sigPresenceProbeToContact(to, from, true);
    }
    else if (presenceType.isEmpty() && !to.isEmpty())
    {
        if (presenceTo.split("/", QString::SkipEmptyParts).count() == 1)
        {
            return Error::generateError(Utils::getHost(presenceTo), "presence", "modify", "jid-malformed", Utils::getHost(presenceTo), presenceTo,
                                        Utils::generateId(), QDomElement());
        }
        else
        {
            QString roomName = Utils::getBareJid(presenceTo);
            QStringList roomTypes = m_mucManager->getRoomTypes(roomName);

            QString show = presence.elementsByTagName("show").item(0).toElement().text();
            QString status = presence.elementsByTagName("status").item(0).toElement().text();

            if (presence.lastChildElement("x").attribute("xmlns") == "http://jabber.org/protocol/muc")
            {
                if (m_mucManager->isLockedRoom(roomName))
                {
                    return Error::generateError(roomName, "presence", "cancel", "item-not-found", presenceTo,
                                                presenceFrom, Utils::generateId(),
                                                document.documentElement().firstChildElement());
                }

                if (m_mucManager->maxOccupantsLimit(roomName))
                {
                    return Error::generateError(roomName, "presence", "wait", "service-unavailable", presenceTo,
                                                presenceFrom, Utils::generateId(),
                                                document.documentElement().firstChildElement());
                }

    //            if (m_mucManager->nicknameOccuped(roomName, presenceTo))
    //            {
    //                return Error::generateError(roomName, "presence", "cancel", "conflict", presenceTo, presenceFrom,
    //                                            Utils::generateId(),
    //                                            document.documentElement().firstChildElement());
    //            }

                if (m_mucManager->isBannedUser(roomName, from))
                {
                    return Error::generateError(roomName, "presence", "auth", "forbidden", presenceTo, presenceFrom,
                                                Utils::generateId(),
                                                document.documentElement().firstChildElement());
                }

                if (roomTypes.contains("membersonly"))
                {
                    if (!m_mucManager->getRoomRegisteredMembersList(roomName).contains(from))
                    {
                        return Error::generateError(roomName, "presence", "auth", "registration-required", presenceTo,
                                                    presenceFrom, Utils::generateId(),
                                                    document.documentElement().firstChildElement());
                    }
                }

                if (roomTypes.contains("passwordprotectedroom"))
                {
                    QString password = presence.elementsByTagName("password").item(0).toElement().text();
                    if (password.isEmpty())
                    {
                        return Error::generateError(roomName, "presence", "auth", "not-authorized", presenceTo, presenceFrom,
                                                    id, presence.firstChildElement());
                    }
                    else
                    {
                        if (m_mucManager->getRoomPassword(roomName) != password)
                        {
                            return Error::generateError(roomName, "presence", "auth", "not-authorized", presenceTo, presenceFrom,
                                                        id, presence.firstChildElement());
                        }
                    }
                }

                // Room does not exist
                if (!m_mucManager->chatRoomExist(roomName))
                {
                    // locked room until user request instant room
                    m_mucManager->createRoom(roomName, Utils::getBareJid(from));
                    m_mucManager->addUserToRoom(roomName, Occupant(presenceFrom, presenceTo, "owner", "moderator", status, show));
                    emit sigMucPresenceBroadCast(from,
                                                 Utils::generatePresence("", presenceTo, from, Utils::generateId(),
                                                                         "owner", "moderator", "", "",
                                                                         QList<int>() << 201 << 110, "", ""));
                    return QByteArray();
                }

                QString occupantMucJid = m_mucManager->getOccupantMucJid(roomName, from);
                if (occupantMucJid.isEmpty() || (occupantMucJid == presenceTo))
                {
                    QString userAffiliation = "member";
                    QString userRole = "participant";
                    if (roomTypes.contains("moderatedroom"))
                        userRole = "visitor";
                    if (m_mucManager->getRoomOwnersList(roomName).contains(from))
                        userAffiliation = "owner";
                    else if (m_mucManager->getRoomAdminsList(roomName).contains(from))
                        userAffiliation = "admin";

                    if (occupantMucJid != presenceTo)
                    {
                        // add user to the room
                        m_mucManager->addUserToRoom(roomName, Occupant(presenceFrom, presenceTo, userAffiliation, "participant", status, show));
                    }

                    QList<Occupant> occupantList = m_mucManager->getOccupants(roomName);
                    foreach (Occupant occupant, occupantList)
                    {
                        QString affiliation = occupant.affiliation();
                        QString role = occupant.role();

                        emit sigMucPresenceBroadCast(presenceFrom, Utils::generatePresence("", occupant.mucJid(),
                                                                                           presenceFrom,
                                                                                           Utils::generateId(),
                                                                                           affiliation, role, "", "",
                                                                                           QList<int>(), occupant.status(),
                                                                                   occupant.show()));

                        if (occupant.jid() != presenceFrom)
                        {
                            QString jid = "";
                            if ((roomTypes.contains("nonanonymous")) ||
                                    ((roomTypes.contains("semianonymous")) && (role == "moderator")))
                                jid = from;
                            emit sigMucPresenceBroadCast(occupant.jid(), Utils::generatePresence("", presenceTo, occupant.jid(),
                                                                                      Utils::generateId(),
                                                                                      userAffiliation, userRole, jid, "",
                                                                                             QList<int>(), status, show));
                        }
                    }

                    // Self presence to the new occupant
                    QList<int> statusCode;
                    statusCode << 110 << 210;
                    if (roomTypes.contains("nonanonymous"))
                        statusCode << 100;
                    if (m_mucManager->loggedDiscussion(roomName))
                        statusCode << 170;
                    emit sigMucPresenceBroadCast(presenceFrom, Utils::generatePresence("", presenceTo,
                                                                                       presenceFrom,
                                                                                       Utils::generateId(),
                                                                                       userAffiliation,
                                                                                       "participant", "", "",
                                                                                     statusCode, status, show));


                    // Room history fetch
                    if (!presence.lastChildElement("x").firstChildElement("history").isNull())
                    {
                        QDomElement historyElement = presence.lastChildElement("x").firstChildElement("history");
                        if (historyElement.attributes().count() == 1)
                        {
                            /*if (historyElement.attribute("maxchar").toInt() != 0)
                            {
                                int maxchar = historyElement.attribute("maxchar").toInt();
                                emit sigRoomHistory(presenceFrom, m_mucManager->getMaxcharsHistory(roomName, maxchar));
                            }
                            else */if (historyElement.attribute("maxstanzas").toInt() != 0)
                            {
                                int maxstanza = historyElement.attribute("maxstanzas").toInt();
                                emit sigRoomHistory(presenceFrom, m_mucManager->getMaxstanzaHistory(roomName, maxstanza));
                            }
                            else if (historyElement.attribute("seconds").toInt() != 0)
                            {
                                int seconds = historyElement.attribute("seconds").toInt();
                                emit sigRoomHistory(presenceFrom, m_mucManager->getLastsecondsHistory(roomName, seconds));
                            }
                            else if (!historyElement.attribute("since").isEmpty())
                            {
                                QString since = historyElement.attribute("since");
                                emit sigRoomHistory(presenceFrom, m_mucManager->getHistorySince(roomName, since));
                            }
                        }
                        else
                        {
                            /*if (!historyElement.attribute("since").isEmpty() &&
                                    !historyElement.attribute("maxchars").isEmpty())
                            {
                                QString since = historyElement.attribute("since");
                                int maxchar = historyElement.attribute("maxchar").toInt();
                                emit sigRoomHistory(presenceFrom, m_mucManager->getHistorySinceMaxchar(roomName, since, maxchar));
                            }
                            else */if (!historyElement.attribute("since").isEmpty() &&
                                     !historyElement.attribute("maxstanzas").isEmpty())
                            {
                                QString since = historyElement.attribute("since");
                                int maxstanza = historyElement.attribute("maxstanzas").toInt();
                                emit sigRoomHistory(presenceFrom, m_mucManager->getHistorySinceMaxstanza(roomName, since, maxstanza));
                            }
//                            else if (!historyElement.attribute("since").isEmpty() &&
//                                     !historyElement.attribute("seconds").isEmpty())
//                            {
//                                QString since = historyElement.attribute("since");
//                                int seconds = historyElement.attribute("seconds").toInt();
//                                emit sigRoomHistory(presenceFrom, m_mucManager->getHistorySinceSeconds(roomName, since, seconds));
//                            }
                        }
                    }
                    else
                    {
                        int maxstanza = m_mucManager->getRoomMaxhistoryFetch(roomName);
                        emit sigRoomHistory(presenceFrom, m_mucManager->getMaxstanzaHistory(roomName, maxstanza));
                    }

                    // send room subject to the new occupant
                    if (occupantMucJid.isEmpty())
                    {
                        // Room subject
                        emit sigRoomSubject(presenceFrom,
                                            generateRoomSubjectMessage(roomName,
                                                                       presenceFrom,
                                                                       Utils::generateId(),
                                                                       m_mucManager->getRoomSubject(roomName)));
                    }
                }
            }
            else
            {
                // changes availability status
                if (!presence.elementsByTagName("show").isEmpty() ||
                        !presence.elementsByTagName("status").isEmpty())
                {
                    Occupant userOccupant = m_mucManager->getOccupantFromMucJid(roomName, presenceTo);
                    if (m_mucManager->canBroadcastPresence(roomName, userOccupant.role()))
                    {
                        QDomElement xElement = document.createElement("x");
                        xElement.setAttribute("xmlns", "http://jabber.org/protocol/muc#user");

                        QDomElement itemElement = document.createElement("item");
                        itemElement.setAttribute("affiliation", userOccupant.affiliation());
                        itemElement.setAttribute("jid", presenceFrom);
                        itemElement.setAttribute("role", userOccupant.role());

                        presence.setAttribute("from", userOccupant.mucJid());

                        xElement.appendChild(itemElement);
                        presence.appendChild(xElement);

                        QList<Occupant> occupantList = m_mucManager->getOccupants(roomName);
                        foreach (Occupant occupant, occupantList)
                        {
                            if (userOccupant.jid() != occupant.jid())
                            {
                                presence.setAttribute("to", occupant.jid());
                                emit sigMucPresenceBroadCast(occupant.jid(), document);
                            }
                        }
                    }

                    // Update user status and role
                    m_mucManager->changeOccupantStatus(roomName, presenceTo,
                                                       presence.elementsByTagName("status").item(0).toElement().text());
                    m_mucManager->changeOccupantShow(roomName, presenceTo,
                                                     presence.elementsByTagName("show").item(0).toElement().text());
                    return QByteArray();
                }

                // Changing nickname in the room
                if (m_mucManager->nicknameOccuped(roomName, presenceTo))
                {
                    QString occupantMucJid = m_mucManager->getOccupantMucJid(roomName, from);
                    return Error::generateError(roomName, "presence", "cancel", "conflict", occupantMucJid,
                                                presenceFrom, Utils::generateId(),
                                                document.documentElement().firstChildElement());
                }

                Occupant userOccupant = m_mucManager->getOccupant(roomName, presenceFrom);

                QList<Occupant> occupantList = m_mucManager->getOccupants(roomName);
                QString nick = Utils::getResource(presenceTo);
                foreach (Occupant occupant, occupantList)
                {
                    QList<int> statusCode;
                    QList<int> statusCode1;
                    statusCode << 303;
                    if (occupant.jid() == presenceFrom)
                    {
                        statusCode << 110;
                        statusCode1 << 110 << 210;
                    }
                    emit sigMucPresenceBroadCast(occupant.jid(),
                                                 Utils::generatePresence("unavailable", userOccupant.mucJid(),
                                                                         occupant.jid(),
                                                                              Utils::generateId(),
                                                                              userOccupant.affiliation(),
                                                                         userOccupant.role(), presenceFrom, nick,
                                                                                     statusCode, "", ""));

                    emit sigMucPresenceBroadCast(occupant.jid(),
                                                 Utils::generatePresence("", presenceTo, occupant.jid(),
                                                                              Utils::generateId(),
                                                                              userOccupant.affiliation(),
                                                                         userOccupant.role(), presenceFrom, "",
                                                                                     statusCode1, "", ""));
                }
                // Change nickname
                m_mucManager->changeRoomNickname(roomName, presenceFrom, nick);
                return QByteArray();
            }
        }
    }
    else // Directed presence
    {
        emit sigDirectedPresence(from, to, document.toByteArray());
    }
    return QByteArray();
}

QByteArray PresenceManager::generateRoomSubjectMessage(QString from, QString to, QString id, QString subject)
{
    QDomDocument document;
    QDomElement messageElement = document.createElement("message");
    messageElement.setAttribute("from", from);
    messageElement.setAttribute("to", to);
    messageElement.setAttribute("id", id);
    messageElement.setAttribute("type", "groupchat");

    QDomElement subjectElement = document.createElement("subject");
    subjectElement.appendChild(document.createTextNode(subject));

    messageElement.appendChild(subjectElement);
    document.appendChild(messageElement);

    return document.toByteArray();
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
