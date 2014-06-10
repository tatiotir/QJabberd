#include "IqManager.h"

/*!
 * \brief IqManager::IqManager constructor of the iq manager
 * \param serverConfigMap
 * \param userManager
 * \param privacyListManager
 * \param rosterManager
 * \param vcardManager
 * \param lastActivityManager
 * \param entityTimeManager
 * \param privateStorageManager
 * \param serviceDiscoveryManager
 * \param offlineMessageManager
 * \param streamNegotiationManager
 * \param oobDataManager
 */
IqManager::IqManager(QJsonObject *serverConfiguration,
                     UserManager *userManager,
                     PrivacyListManager *privacyListManager, RosterManager *rosterManager,
                     VCardManager *vcardManager, LastActivityManager *lastActivityManager,
                     EntityTimeManager *entityTimeManager, PrivateStorageManager *privateStorageManager,
                     ServiceDiscoveryManager *serviceDiscoveryManager,
                     OfflineMessageManager *offlineMessageManager,
                     StreamNegotiationManager *streamNegotiationManager, BlockingCommandManager *blockingCmdManager,
                     MucManager *mucManager, ByteStreamsManager *byteStreamManager)
{
    m_serverConfiguration = serverConfiguration;
    m_userManager = userManager;
    m_privacyListManager = privacyListManager;
    m_vCardManager = vcardManager;
    m_lastActivityManager = lastActivityManager;
    m_entityTimeManager = entityTimeManager;
    m_rosterManager = rosterManager;
    m_privateStorageManager = privateStorageManager;
    m_serviceDiscoveryManager = serviceDiscoveryManager;
    m_offlineMessageManager = offlineMessageManager;
    m_streamNegotiationManager = streamNegotiationManager;
    m_blockingCmdManager = blockingCmdManager;
    m_mucManager = mucManager;
    m_byteStreamManager = byteStreamManager;
}

/*!
 * \brief The IqManager::authenticate method authenticate an user using the old jabber:iq:auth mechanism
 * \param streamId
 * \param id
 * \param username
 * \param password
 * \param resource
 * \param digest
 * \param host
 * \return QByteArray
 */
QByteArray IqManager::authenticate(QString streamId, QString id, QString username, QString password,
                                   QString resource, QString digest, QString host)
{
    if (username.isEmpty() || resource.isEmpty())
    {
        return Error::generateError("", "iq", "modify", "not-acceptable", "", "", id, QDomElement());
    }

    if (!digest.isEmpty())
    {
        QString jid = username + '@' + host;
        QString password = m_userManager->getPassword(jid);

        QString userDigest = Utils::digestCalculator(id, password);
        if ((userDigest != digest))
        {
            Error::generateError("", "iq", "auth", "not-authorized", "", "", id, QDomElement());
        }
        else
        {
            emit sigNonSaslAuthentification(streamId, jid + "/" + resource, id);
            return QByteArray();
        }
    }
    else
    {
        QString jid = username + '@' + host;
        QString userPassword = m_userManager->getPassword(jid);

        if (userPassword != password)
        {
            return Error::generateError("", "iq", "auth", "not-authorized", "", "", id, QDomElement());
        }
        else
        {
            emit sigNonSaslAuthentification(streamId, jid + "/" + resource, id);
            return QByteArray();
        }
    }
    return QByteArray();
}


/*!
 * \brief The IqManager::authentificationFields method return the authentification fields to an user which want to use the old jabber authentification mechanism (v1.0)
 * \param id
 * \return QByteArray
 */
QByteArray IqManager::authentificationFields(QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");

    if (id != "")
    {
        iq.setAttribute("id", id);
    }

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:auth");

    query.appendChild(document.createElement("username"));
    query.appendChild(document.createElement("password"));
    query.appendChild(document.createElement("digest"));
    query.appendChild(document.createElement("ressouces"));

    iq.appendChild(query);
    document.appendChild(iq);

    return document.toByteArray();
}

/*!
 * \brief The IqManager::parseIQ method parse an iq xml request and output the result in a byte array
 * \param document
 * \param from
 * \param host
 * \param streamId
 * \return QByteArray
 */
QByteArray IqManager::parseIQ(QDomDocument document, QString from, QString host, QString streamId)
{
    QDomElement iq = document.firstChild().toElement();
    QString iqFrom = iq.attribute("from", from);
    QString iqTo = iq.attribute("to");
    QString id = iq.attribute("id", Utils::generateId());

    // Checking for privacy list
    QByteArray privacyListError = m_privacyListManager->isBlocked(iqFrom, iqTo, "iq");
    if (!privacyListError.isEmpty())
        return privacyListError;

    QByteArray privacyListError1 = m_privacyListManager->isBlocked(iqTo, iqFrom, "iq");
    if (!privacyListError1.isEmpty())
        return privacyListError1;

    // Checking for block list
    QByteArray blockListError = m_blockingCmdManager->isBlocked(iqFrom, iqTo, "iq");
    if (!blockListError.isEmpty())
        return blockListError;

    if (iq.attribute("type") == "set")
    {
        QDomElement firstChild = iq.firstChildElement();
        QString firstChildTagName = firstChild.tagName();

        if (firstChildTagName == "session")
        {
            return generateIqSessionReply(id, Utils::getHost(iqFrom));
        }
        // Resource binding
        else if (firstChildTagName == "bind")
        {
            QDomElement bindChild = document.documentElement().firstChild().toElement();
            QString fullJid;
            QString resource;

            // Empty resource binding
            if (bindChild.text().isEmpty())
            {
                resource = Utils::generateResource();
                fullJid = m_streamNegotiationManager->getUserJid(streamId) + "/" + resource;
            }
            else
            {
                resource = bindChild.firstChild().toElement().text();
                fullJid = m_streamNegotiationManager->getUserJid(streamId) + "/" + resource;
            }

            emit sigResourceBinding(streamId, fullJid, id);
            return QByteArray();
        }
        else if (firstChildTagName == "query")
        {
            QString xmlns = firstChild.attribute("xmlns");

            if ((xmlns == "jabber:iq:auth") && m_serverConfiguration->value("modules").toObject().value("nonsaslauth").toBool())
            {
                return authenticate(streamId, id,
                                    document.documentElement().elementsByTagName("username").item(0).toElement().text(),
                                    document.documentElement().elementsByTagName("password").item(0).toElement().text(),
                                    document.documentElement().elementsByTagName("resource").item(0).toElement().text(),
                                    document.documentElement().elementsByTagName("digest").item(0).toElement().text(),
                                    document.documentElement().attribute("to"));
            }
            if ((xmlns == "jabber:iq:roster") && m_serverConfiguration->value("modules").toObject().value("roster").toBool())
            {
                // We check if there are errors.
                QDomNodeList groupNodes = firstChild.firstChild().toElement().elementsByTagName("group");
                for (int i = 0; i < groupNodes.count(); ++i)
                {
                    // if one of the group node text is empty
                    QDomElement groupElement = groupNodes.item(i).toElement();
                    if ((groupElement.text().isEmpty()) || (groupElement.text().count() > 100))
                    {
                        return Error::generateError("", "iq", "modify", "not-acceptable", iqFrom, "", id, QDomElement());
                    }
                }

                if (firstChild.elementsByTagName("item").count() > 1)
                {
                    return Error::generateError("", "iq", "modify", "bad-request", iqFrom, "", id, QDomElement());
                }

                // the name attribute is too long
                if (firstChild.firstChild().toElement().attribute("name").count() > 100)
                {
                    return Error::generateError("", "iq", "modify", "not-acceptable", iqFrom, "", id, QDomElement());
                }
                // Unauthorized entity

                QDomElement item = firstChild.firstChild().toElement();

                QString jid = item.attribute("jid");
                QString name = item.attribute("name");
                QString subscription = item.attribute("subscription", "none");
                QString ask = item.attribute("ask", "");
                QString version = item.attribute("version");

                if (subscription == "remove")
                {
                    if (!m_rosterManager->contactExists(Utils::getBareJid(iqFrom), jid))
                    {
                        return Error::generateError("", "iq", "cancel", "item-not-found", iqFrom, "", id, QDomElement());
                    }

                    Contact userContact = m_rosterManager->getContact(Utils::getBareJid(iqFrom), jid);
                    if (m_rosterManager->deleteContactToRoster(Utils::getBareJid(iqFrom), jid))
                    {
                        // Roster push
                        emit sigRosterPush(iqFrom, Utils::generateRosterPush(iqFrom, id,
                                                                             userContact.getJid(),
                                                                             userContact.getName(),
                                                                             "remove",
                                                                             userContact.getAsk(),
                                                                             userContact.getApproved(),
                                                                             userContact.getGroups()));

                        if ((userContact.getSubscription() == "to"))
                        {
                            emit sigPresenceBroadCast(jid, Utils::generatePresence("unsubscribe",
                                                                                   Utils::getBareJid(iqFrom),
                                                                                   jid,
                                                                                   Utils::generateId(),
                                                                                   "", "", ""));
                        }

                        Contact contactUser = m_rosterManager->getContact(jid, Utils::getBareJid(iqFrom));
                        if ((contactUser.getSubscription() == "to"))
                        {
                            emit sigPresenceBroadCast(jid, Utils::generatePresence("unsubscribed",
                                                                                   Utils::getBareJid(iqFrom),
                                                                                   jid,
                                                                                   Utils::generateId(),
                                                                                   "", "", ""));
                        }

                        // Mutual subscription
                        if (userContact.getSubscription() == "both")
                        {
                            emit sigPresenceBroadCast(jid, Utils::generatePresence("unsubscribe",
                                                                                   Utils::getBareJid(iqFrom),
                                                                                   jid,
                                                                                   Utils::generateId(),
                                                                                   "", "", ""));

                            emit sigPresenceBroadCast(jid, Utils::generatePresence("unsubscribed",
                                                                                   Utils::getBareJid(iqFrom),
                                                                                   jid,
                                                                                   Utils::generateId(),
                                                                                   "", "", ""));

                        }
                        return generateIQResult("", iqFrom, id);
                    }
                }
                else
                {
                    QDomNodeList groupNodes = item.elementsByTagName("group");
                    QSet<QString> groupsName;
                    for(int i = 0; i < groupNodes.count(); ++i)
                    {
                        groupsName << groupNodes.item(i).toElement().text();
                    }

                    if (!jid.isEmpty())
                    {
                        Contact userContact(version, false, ask, jid, name, subscription, groupsName);
                        if (m_rosterManager->addContactToRoster(Utils::getBareJid(iqFrom), userContact))
                        {
                            Contact userContact = m_rosterManager->getContact(Utils::getBareJid(iqFrom), jid);
                            // we emit the signal roster push to the stream manager
                            emit sigRosterPush(Utils::getBareJid(iqFrom),
                                               Utils::generateRosterPush(iqFrom,
                                                                         id, jid,
                                                                         userContact.getName(),
                                                                         userContact.getSubscription(),
                                                                         userContact.getAsk(),
                                                                         userContact.getApproved(),
                                                                         userContact.getGroups()));

                            return generateIQResult("", iqFrom, id);
                        }
                        else
                        {
                            return Error::generateInternalServerError();
                        }
                    }
                }
            }
            else if ((xmlns == "jabber:iq:register") && m_serverConfiguration->value("modules").toObject().value("register").toBool())
            {
                // Registering with room
                if (m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(Utils::getHost(iqTo)))
                {
                    QMap<QString, QVariant> dataFormValue = DataFormManager::parseDataForm(
                                firstChild.firstChild().toElement());

                    if (dataFormValue.value("muc#register_roomnick").toString().isEmpty())
                    {
                        return Error::generateError("", "iq", "modify", "bad-request",
                                                    iqTo, iqFrom, id, QDomElement());
                    }

                    QString mucJid = iqTo + "/" + dataFormValue.value("muc#register_roomnick").toString();
                    if (m_mucManager->nicknameOccuped(iqTo, mucJid))
                    {
                        return Error::generateError("", "iq", "cancel", "conflict", iqTo, iqFrom,
                                                    id, QDomElement());
                    }

                    // Registered user
                    if (m_mucManager->registerUser(iqTo, Occupant(iqFrom, mucJid, "member", "participant", "", "")))
                    {
                        QList<Occupant> occupantList = m_mucManager->getOccupants(iqTo);
                        foreach (Occupant occupant, occupantList)
                        {
                            emit sigMucPresenceBroadCast(occupant.jid(), Utils::generatePresence("", mucJid, occupant.jid(), Utils::generateId(), "member",
                                                                                "participant", iqFrom, "", QList<int>(), "", ""));

                        }
                        return generateIQResult(iqTo, iqFrom, id);
                    }
                    return QByteArray();
                }

                if (firstChild.firstChildElement().tagName() != "x")
                {
                    QString username = firstChild.elementsByTagName("username").item(0).toElement().text();
                    QString password = firstChild.elementsByTagName("password").item(0).toElement().text();
                    QString jid = username + "@" + host;

                    return registerUserReply(username, password, jid, id, firstChild, iqFrom);
                }
                // Registration using data form
                else
                {
                    QMap<QString, QVariant> dataFormValue = DataFormManager::parseDataForm(firstChild.firstChildElement());

                    QString formType = dataFormValue.value("FORM_TYPE").toString();
                    if (formType == "jabber:iq:register")
                    {
                        QString username = dataFormValue.value("username").toString();
                        QString password = dataFormValue.value("password").toString();
                        QString jid = username + "@" + host;

                        return registerUserReply(username, password, jid, id, firstChild, iqFrom);
                    }
                    else if (formType == "jabber:iq:register:changepassword")
                    {
                        QString username = dataFormValue.value("username").toString();
                        QString password = dataFormValue.value("password").toString();
                        QString jid = username + "@" + host;

                        if (m_userManager->changePassword(jid, password))
                        {
                            return generateIQResult("", "", id);
                        }
                        else
                        {
                            return Error::generateInternalServerError();
                        }
                    }
                }
            }
            else if (firstChild.firstChild().toElement().tagName() == "remove")
            {
                if (firstChild.elementsByTagName("remove").count() > 1)
                {
                    return Error::generateError("", "iq", "modify", "bad-request", Utils::getHost(iqFrom), "", id, QDomElement());
                }
                // We verify others errors

                if (m_userManager->deleteUser(Utils::getBareJid(iqFrom)))
                {
                    return generateIQResult("", iqFrom, id);
                }
                else
                {
                    return Error::generateInternalServerError();
                }
            }
            else if ((xmlns == "jabber:iq:privacy") && m_serverConfiguration->value("modules").toObject().value("privacy").toBool())
            {
                return m_privacyListManager->privacyListReply(document, iqFrom);
            }
            else if ((xmlns == "jabber:iq:private") && m_serverConfiguration->value("modules").toObject().value("private").toBool())
            {
                return m_privateStorageManager->privateStorageManagerReply(document, iqFrom);
            }
            else if ((xmlns == "jabber:iq:oob") && m_serverConfiguration->value("modules").toObject().value("oob").toBool())
            {
                emit sigApplicationRequest(iqTo, document);
                return QByteArray();
            }
            else if (xmlns == "http://jabber.org/protocol/muc#owner")
            {
                if (firstChild.firstChildElement().tagName() == "destroy")
                {
                    QString roomName = iqTo;
                    if (!m_mucManager->getRoomOwnersList(roomName).contains(Utils::getBareJid(iqFrom)))
                    {
                        return Error::generateError("", "iq", "auth", "forbidden", iqTo, iqFrom,
                                                    Utils::generateId(), firstChild);
                    }
                    else
                    {
                        //QString ownerMucJid = m_mucManager->getOccupantMucJid(roomName, iqFrom);
                        QList<Occupant> occupantList = m_mucManager->getOccupants(roomName);

                        // Destroy the room
                        m_mucManager->destroyRoom(roomName);

                        foreach (Occupant occupant, occupantList)
                        {
                            QDomDocument presenceDocument = Utils::generatePresence("unavailable", occupant.mucJid(), occupant.jid(),
                                                                                    Utils::generateId(), "none",
                                                                                    "none", "", "",
                                                                                    QList<int>(), "", "");
                            presenceDocument.firstChildElement().appendChild(firstChild.firstChildElement());
                            emit sigMucPresenceBroadCast(occupant.jid(), presenceDocument);
                        }
                    }
                    return generateIQResult(iqTo, iqFrom, id);
                }

                QDomElement xElement = firstChild.firstChildElement();
                if (!xElement.hasChildNodes())
                {
                    if ((xElement.attribute("xmlns") == "jabber:xdata") && (xElement.attribute("type") == "submit"))
                    {
                        m_mucManager->unlockRoom(iqTo);
                    }
                }
                else
                {
                    QMap<QString, QVariant> dataFormValue = DataFormManager::parseDataForm(xElement);

                    if (((dataFormValue.value("muc#roomconfig_passwordprotectedroom").toBool()) &&
                            dataFormValue.value("muc#roomconfig_roomsecret").toString().isEmpty()) ||
                            ((!dataFormValue.value("muc#roomconfig_passwordprotectedroom").toBool()) &&
                                                        !dataFormValue.value("muc#roomconfig_roomsecret").toString().isEmpty()))
                    {
                        return Error::generateError("", "iq", "cancel", "not-acceptable",
                                                         iqTo, iqFrom, id, QDomElement());
                    }

                    if (m_mucManager->submitConfigForm(iqTo, dataFormValue))
                    {
                        // Unlock the room
                        if (m_mucManager->isLockedRoom(iqTo))
                            m_mucManager->unlockRoom(iqTo);

                        QList<Occupant> occupantList = m_mucManager->getOccupants(iqTo);
                        foreach (Occupant occupant, occupantList)
                        {
                            emit sigGroupchatMessage(occupant.jid(), Utils::generateMucNotificationMessage("groupchat", iqTo, occupant.jid(),
                                                                                                           Utils::generateId(), QList<int>()));
                        }
                        return generateIQResult(iqTo, iqFrom, id);
                    }
                }
            }
            else if (xmlns == "http://jabber.org/protocol/muc#admin")
            {

                QDomNodeList itemList = firstChild.childNodes();
                for (int i = 0; i < itemList.count(); ++i)
                {
                    QString nick = itemList.item(i).toElement().attribute("nick");
                    QString mucJid = iqTo + "/" + nick;
                    QString role = itemList.item(i).toElement().attribute("role");
                    QString affiliation = itemList.item(i).toElement().attribute("affiliation");

                    if (!role.isEmpty())
                    {
                        // kicking himself
                        if (mucJid == m_mucManager->getOccupantMucJid(iqTo, iqFrom))
                        {
                            return Error::generateError("", "iq", "cancel", "conflict", iqTo, iqFrom,
                                                        id, QDomElement());
                        }

                        Occupant kickerOccupant = m_mucManager->getOccupant(iqTo, iqFrom);
                        Occupant kickedOccupant = m_mucManager->getOccupantFromMucJid(iqTo, mucJid);

                        // Can't kick user with higher affiliation
                        if (Utils::affiliationIntValue(kickerOccupant.affiliation())
                                < Utils::affiliationIntValue(kickedOccupant.affiliation()))
                        {
                            return Error::generateError("", "iq", "cancel", "not-allowed", iqTo, iqFrom, id,
                                                        QDomElement());
                        }

                        // kicking an occupant
                        if (role == "none")
                        {
                            QString kickedJid = m_mucManager->getOccupantJid(iqTo, mucJid);
                            emit sigMucPresenceBroadCast(kickedJid, Utils::generatePresence("unavailable", mucJid, kickedJid, "", "none", "none",
                                                                                            "", "", QList<int>() << 307, "", ""));
                        }

                        // Update role
                        m_mucManager->changeRole(iqTo, mucJid, role);

                        QList<Occupant> occupantList = m_mucManager->getOccupants(iqTo);

                        QDomDocument document;
                        // Informs Remaning occupant
                        if (role == "none")
                        {
                            document = Utils::generatePresence("unavailable", mucJid, mucJid,
                                                               Utils::generateId(), "none", "none", "", "",
                                                               QList<int>() << 307, "", "");
                        }
                        else if (role == "participant")
                        {
                            document = Utils::generatePresence("", mucJid, mucJid, Utils::generateId(),
                                                               kickedOccupant.affiliation(),
                                                               "participant", "", nick,
                                                               QList<int>(), "", "");
                        }
                        else if (role == "visitor")
                        {
                            document = Utils::generatePresence("", mucJid, mucJid, Utils::generateId(),
                                                               kickedOccupant.affiliation(), "visitor",
                                                               kickedOccupant.jid(),
                                                               "", QList<int>(), "", "");
                        }
                        else if (role == "moderator")
                        {
                            document = Utils::generatePresence("", mucJid, mucJid, Utils::generateId(),
                                                               kickerOccupant.affiliation(),
                                                               "moderator", kickerOccupant.jid(),
                                                               "", QList<int>(), "", "");
                        }

                        foreach (Occupant occupant, occupantList)
                        {
                            document.firstChildElement().setAttribute("to", occupant.jid());
                            emit sigMucPresenceBroadCast(occupant.jid(), document);
                        }

                        return generateIQResult(iqTo, iqFrom, id);
                    }
                    else if (!affiliation.isEmpty())
                    {
                        QString userJid = itemList.item(i).toElement().attribute("jid");

                        QList<QString> ownersList = m_mucManager->getRoomOwnersList(iqTo);

                        // Only owners can modify affiliation for an user
                        if (!ownersList.contains(Utils::getBareJid(iqFrom)))
                        {
                            return Error::generateError("", "iq", "auth", "forbidden", iqTo, iqFrom,
                                                        Utils::generateId(), QDomElement());
                        }


                        // There is no other owner than this user
                        if ((ownersList.count() == 1) && (ownersList.contains(Utils::getBareJid(iqFrom)))
                                && (affiliation != "owner"))
                        {
                            return Error::generateError("", "iq", "cancel", "conflict", iqTo, iqFrom,
                                                        id, QDomElement());
                        }

                        // Change occupant affiliation
                        m_mucManager->changeAffiliation(iqTo, userJid, affiliation);
                        if (affiliation == "member")
                        {
                            // Send invitation to this user
                            emit sigGroupchatMessage(userJid,
                                                     Utils::generateMucInvitationMessage(iqTo, userJid,
                                                                                         Utils::generateId(),
                                                                                         Utils::getBareJid(iqFrom),
                                                                                         m_mucManager->getRoomPassword(iqTo), ""));


                            QList<Occupant> userOccupantList = m_mucManager->getOccupants(iqTo, Utils::getBareJid(userJid));
                            if (!userOccupantList.isEmpty())
                            {
                                QList<Occupant> occupantsList = m_mucManager->getOccupants(iqTo);
                                foreach (Occupant occupant, occupantsList)
                                {
                                    foreach (Occupant userOccupant, userOccupantList)
                                    {
                                        emit sigMucPresenceBroadCast(occupant.jid(),
                                                                     Utils::generatePresence("", userOccupant.mucJid(), occupant.jid(),
                                                                                             "", "member", "", userOccupant.jid(),
                                                                                             "", QList<int>(), "", ""));
                                    }
                                }
                            }
                        }
                        else if (affiliation == "admin")
                        {
                            QList<Occupant> userOccupantList = m_mucManager->getOccupants(iqTo, Utils::getBareJid(userJid));
                            if (!userOccupantList.isEmpty())
                            {
                                QList<Occupant> occupantsList = m_mucManager->getOccupants(iqTo);
                                foreach (Occupant occupant, occupantsList)
                                {
                                    foreach (Occupant userOccupant, userOccupantList)
                                    {
                                        emit sigMucPresenceBroadCast(occupant.jid(),
                                                                     Utils::generatePresence("", userOccupant.mucJid(),
                                                                                             occupant.jid(),
                                                                                             "", "admin",
                                                                                             userOccupant.role(),
                                                                                             userOccupant.jid(), "", QList<int>(), "", ""));
                                    }
                                }
                            }
                            else
                            {
                                QList<Occupant> occupantsList = m_mucManager->getOccupants(iqTo);
                                foreach (Occupant occupant, occupantsList)
                                {
                                    emit sigMucPresenceBroadCast(occupant.jid(),
                                                                 Utils::generatePresence("", iqTo, occupant.jid(),
                                                                                         "", "admin", "", userJid,
                                                                                         "", QList<int>(), "", ""));
                                }
                            }
                        }
                        else if (affiliation == "none")
                        {
                            if (m_mucManager->getRoomTypes(iqTo).contains("membersonly"))
                            {
                                QList<Occupant> removedOccupantList = m_mucManager->getOccupants(iqTo, Utils::getBareJid(userJid));

                                // Remove banned user to the occupants list
                                m_mucManager->removeOccupants(iqTo, Utils::getBareJid(userJid));

                                QList<Occupant> remaningOccupantList = m_mucManager->getOccupants(iqTo);
                                foreach (Occupant remaningOccupant, remaningOccupantList)
                                {
                                    foreach (Occupant removedOccupant, removedOccupantList)
                                    {
                                        emit sigMucPresenceBroadCast(remaningOccupant.jid(),
                                                                     Utils::generatePresence("unavailable",
                                                                                             removedOccupant.mucJid(),
                                                                                             remaningOccupant.jid(), "",
                                                                                             "none", "none", "",
                                                                                             "", QList<int>() << 321,
                                                                                             "", ""));
                                    }
                                }
                            }
                            else
                            {
                                QList<Occupant> userOccupantList = m_mucManager->getOccupants(iqTo, Utils::getBareJid(userJid));
                                if (!userOccupantList.isEmpty())
                                {
                                    QList<Occupant> occupantsList = m_mucManager->getOccupants(iqTo);
                                    foreach (Occupant occupant, occupantsList)
                                    {
                                        foreach (Occupant userOccupant, userOccupantList)
                                        {
                                            emit sigMucPresenceBroadCast(occupant.jid(),
                                                                         Utils::generatePresence("", userOccupant.mucJid(),
                                                                                                 occupant.jid(),
                                                                                                 "", "none",
                                                                                                 userOccupant.role(),
                                                                                                 userOccupant.jid(), "", QList<int>(), "", ""));
                                        }
                                    }
                                }
                                else
                                {
                                    QList<Occupant> occupantsList = m_mucManager->getOccupants(iqTo);
                                    foreach (Occupant occupant, occupantsList)
                                    {
                                        emit sigMucPresenceBroadCast(occupant.jid(),
                                                                     Utils::generatePresence("", iqTo, occupant.jid(),
                                                                                             "", "none", "", userJid,
                                                                                             "", QList<int>(), "", ""));
                                    }
                                }
                            }
                        }
                        else if (affiliation == "outcast")
                        {
                            QList<Occupant> bannedOccupantList = m_mucManager->getOccupants(iqTo, Utils::getBareJid(userJid));
                            foreach (Occupant bannedOccupant, bannedOccupantList)
                            {
                                emit sigMucPresenceBroadCast(bannedOccupant.jid(),
                                                             Utils::generatePresence("unavailable",
                                                                                     bannedOccupant.mucJid(),
                                                                                     bannedOccupant.jid(), "",
                                                                                     "outcast", "none", "",
                                                                                     "", QList<int>() << 301,
                                                                                     "", ""));
                            }

                            // Remove banned user to the occupants list
                            m_mucManager->removeOccupants(iqTo, Utils::getBareJid(userJid));

                            QList<Occupant> remaningOccupantList = m_mucManager->getOccupants(iqTo);
                            foreach (Occupant remaningOccupant, remaningOccupantList)
                            {
                                foreach (Occupant bannedOccupant, bannedOccupantList)
                                {
                                    emit sigMucPresenceBroadCast(remaningOccupant.jid(),
                                                                 Utils::generatePresence("unavailable",
                                                                                         bannedOccupant.mucJid(),
                                                                                         remaningOccupant.jid(), "",
                                                                                         "outcast", "none", "",
                                                                                         "", QList<int>() << 301,
                                                                                         "", ""));
                                }
                            }
                        }
                        else if (affiliation == "owner")
                        {
                            QList<Occupant> userOccupantList = m_mucManager->getOccupants(iqTo, Utils::getBareJid(userJid));
                            if (!userOccupantList.isEmpty())
                            {
                                QList<Occupant> occupantsList = m_mucManager->getOccupants(iqTo);
                                foreach (Occupant occupant, occupantsList)
                                {
                                    foreach (Occupant userOccupant, userOccupantList)
                                    {
                                        emit sigMucPresenceBroadCast(occupant.jid(),
                                                                     Utils::generatePresence("", userOccupant.mucJid(),
                                                                                             occupant.jid(),
                                                                                             "", "owner",
                                                                                             userOccupant.role(),
                                                                                             userOccupant.jid(), "", QList<int>(), "", ""));
                                    }
                                }
                            }
                            else
                            {
                                QList<Occupant> occupantsList = m_mucManager->getOccupants(iqTo);
                                foreach (Occupant occupant, occupantsList)
                                {
                                    emit sigMucPresenceBroadCast(occupant.jid(),
                                                                 Utils::generatePresence("", iqTo, occupant.jid(),
                                                                                         "", "owner", "", userJid,
                                                                                         "", QList<int>(), "", ""));
                                }
                            }
                        }
                        return generateIQResult(iqTo, iqFrom, id);
                    }
                }
            }
            else if (xmlns == "storage:client:avatar")
            {
                QString nodeData;
                QTextStream stream(&nodeData);

                QMultiHash<QString, QString> nodeMap;

                firstChild.firstChildElement().save(stream, 4);
                nodeMap.insert(xmlns, nodeData);
                m_privateStorageManager->storePrivateData(Utils::getBareJid(iqFrom), nodeMap);
                return QByteArray();
            }
            else if ((xmlns == "http://jabber.org/protocol/bytestreams") && m_serverConfiguration->value("modules").toObject().value("bytestreams").toBool())
            {
                emit sigApplicationRequest(iqTo, document);
                return QByteArray();
            }
        }
        else if (((firstChildTagName == "vCard") || (firstChildTagName == "VCARD")) && m_serverConfiguration->value("modules").toObject().value("vcard-temp").toBool())
        {
            return m_vCardManager->vCardManagerReply(document, iqFrom);
        }
        else if (firstChildTagName == "offline" && m_serverConfiguration->value("modules").toObject().value("msgoffline").toBool())
        {
            return m_offlineMessageManager->offlineMessageManagerReply(document, iqFrom);
        }
        else if ((firstChildTagName == "block") && m_serverConfiguration->value("modules").toObject().value("blockingcmd").toBool())
        {
            return m_blockingCmdManager->blockingCommandManagerReply(document, iqFrom);
        }
        else if ((firstChildTagName == "unblock") && m_serverConfiguration->value("modules").toObject().value("blockingcmd").toBool())
        {
            return m_blockingCmdManager->blockingCommandManagerReply(document, iqFrom);
        }
        else if (firstChildTagName == "env:Envelope")
        {
            emit sigApplicationRequest(iqTo, document);
            return QByteArray();
        }
        else if (firstChildTagName == "si")
        {
            document.firstChildElement().setAttribute("from", iqFrom);
            emit sigApplicationRequest(iqTo, document);
            return QByteArray();
        }
        else if ((firstChildTagName == "open") || (firstChildTagName == "close") || (firstChildTagName == "data"))
        {
            if ((firstChild.attribute("xmlns") == "http://jabber.org/protocol/ibb") && m_serverConfiguration->value("modules").toObject().value("ibb").toBool())
            {
                emit sigApplicationRequest(iqTo, document);
                return QByteArray();
            }
        }
        else if (firstChildTagName == "feature")
        {
            emit sigApplicationRequest(iqTo, document);
            return QByteArray();
        }
        else
        {
            return Error::generateError("", "iq", "cancel", "service-unavailable",
                                        Utils::getHost(iqFrom), iqFrom, "", QDomElement());
        }
    }
    else if (iq.attribute("type") == "get")
    {
        QDomElement firstChild = iq.firstChildElement();
        QString firstChildTagName = firstChild.tagName();

        if (firstChildTagName == "query")
        {
            QString xmlns = firstChild.attribute("xmlns");
            if ((xmlns == "jabber:iq:auth") && m_serverConfiguration->value("modules").toObject().value("nonsaslauth").toBool())
            {
                if (m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(document.documentElement().attribute("to")))
                {
                    emit sigStreamNegotiationError(streamId);
                    return Error::generateStreamError("host-unknown");
                }
                return authentificationFields(id);
            }
            else if ((xmlns == "jabber:iq:roster") && m_serverConfiguration->value("modules").toObject().value("roster").toBool())
            {
                if (firstChild.text().isEmpty())
                {
                    QList<Contact> contactList = m_rosterManager->getContactsList(Utils::getBareJid(iqFrom));
                    return generateRosterGetResultReply(iqFrom, id, contactList);
                }
            }
            else if ((xmlns == "jabber:iq:register") && m_serverConfiguration->value("modules").toObject().value("register").toBool())
            {
                // Registering with room
                if (m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(Utils::getHost(iqTo)))
                {
                    if (!m_mucManager->chatRoomExist(iqTo))
                    {
                        return Error::generateError("", "iq", "cancel", "item-not-found",
                                                    iqTo, iqFrom, id, QDomElement());
                    }

                    if (m_mucManager->getOccupantAffiliation(iqTo, iqFrom) != "admin")
                    {
                        return Error::generateError("", "iq", "cancel", "not-allowed", iqTo, iqFrom, id,
                                                    QDomElement());
                    }

                    if (m_mucManager->isRegistered(iqTo, Utils::getBareJid(iqFrom)))
                    {
                        QDomDocument document;
                        QDomElement iqElement = document.createElement("iq");
                        iqElement.setAttribute("from", iqTo);
                        iqElement.setAttribute("to", iqFrom);

                        QDomElement query = document.createElement("query");
                        query.setAttribute("xmlns", "jabber:iq:register");
                        query.appendChild(document.createElement("registered"));

                        QDomElement usernameElement = document.createElement("username");
                        usernameElement.appendChild(document.createTextNode(m_mucManager->getOccupantMucJid(iqTo, iqFrom).split("/").value(1)));

                        query.appendChild(usernameElement);
                        iqElement.appendChild(query);
                        document.appendChild(iqElement);

                        return document.toByteArray();
                    }

                    // Return registration form
                    return DataFormManager::getRoomRegistrationForm(iqTo, iqFrom, id, m_mucManager->getRoomName(iqTo)).toByteArray();
                }
                return DataFormManager::getRegistrationForm(id).toByteArray();
            }
            else if (xmlns.contains("http://jabber.org/protocol/disco") && m_serverConfiguration->value("modules").toObject().value("disco").toBool())
            {
                if (xmlns == "http://jabber.org/protocol/disco#info")
                {
                    return m_serviceDiscoveryManager->serviceDiscoveryManagerReply(document, iqFrom);
                }
                else if (xmlns == "http://jabber.org/protocol/disco#items")
                {
                    return m_serviceDiscoveryManager->serviceDiscoveryManagerReply(document, iqFrom);
                }
            }
            else if ((xmlns == "jabber:iq:privacy") && m_serverConfiguration->value("modules").toObject().value("privacy").toBool())
            {
                return m_privacyListManager->privacyListReply(document, iqFrom);
            }
            else if ((xmlns == "jabber:iq:last") && m_serverConfiguration->value("modules").toObject().value("lastActivity").toBool())
            {
                return m_lastActivityManager->lastActivityReply(document, iqFrom);
            }
            else if ((xmlns == "jabber:iq:private") && m_serverConfiguration->value("modules").toObject().value("private").toBool())
            {
                return m_privateStorageManager->privateStorageManagerReply(document, iqFrom);
            }
            else if (xmlns == "http://jabber.org/protocol/muc#owner")
            {
                if (!m_mucManager->getRoomOwnersList(iqTo).contains(Utils::getBareJid(iqFrom)))
                {
                    return Error::generateError("", "iq", "auth", "forbidden", iqTo, iqFrom,
                                                Utils::generateId(), firstChild);
                }
                else
                {
                    return DataFormManager::getRoomConfigForm("form", iqTo, iqFrom, id, m_mucManager->getRoomConfig(iqTo)).toByteArray();
                }
            }
            else if (xmlns == "http://jabber.org/protocol/muc#admin")
            {
                QString roomName = iqTo;
                if ((m_mucManager->getRoomTypes(roomName).contains("membersonly")) &&
                        !m_mucManager->getRoomAdminsList(roomName).contains(Utils::getBareJid(iqFrom)))
                {
                    return Error::generateError("", "iq", "cancel", "not-allowed", iqTo, iqFrom, id,
                                                QDomElement());
                }
                else
                {
                    QString affiliation = firstChild.firstChildElement().attribute("affiliation");
                    QString role = firstChild.firstChildElement().attribute("role");

                    if (!affiliation.isEmpty())
                    {
                        if (affiliation == "owner")
                        {
                            return generateRoomAffiliationList(iqTo, iqFrom, id,
                                                               m_mucManager->getRoomOwnersList(iqTo), "owner");
                        }
                        else if (affiliation == "admin")
                        {
                            return generateRoomAffiliationList(iqTo, iqFrom, id,
                                                               m_mucManager->getRoomAdminsList(iqTo), "admin");
                        }
                        else if (affiliation == "member")
                        {
                            return generateRoomAffiliationList(iqTo, iqFrom, id, m_mucManager->getRoomRegisteredMembersList(iqTo), "member");
                        }
                        else if (affiliation == "outcast")
                        {
                            return generateRoomAffiliationList(iqTo, iqFrom, id,
                                                               m_mucManager->getBannedList(iqTo), "outcast");
                        }
                    }
                    else
                    {
                        if (!role.isEmpty())
                        {
                            if (role == "moderator")
                            {
                                return generateRoomRoleList(iqTo, iqFrom, id,
                                                                   m_mucManager->getRoomModeratorsJid(iqTo), "moderator");
                            }
                        }
                    }
                }
            }
            else if ((xmlns == "jabber:iq:avatar"))
            {
                document.firstChildElement().setAttribute("from", iqFrom);
                emit sigIqAvatarQuery(iqTo, document);
                return QByteArray();
            }
            else if (xmlns == "storage:client:avatar")
            {
                QDomDocument document;
                document.setContent(m_privateStorageManager->getPrivateData(Utils::getBareJid(iqTo),
                                                                           "storage:client:avatar"));
                emit sigIqAvatarQuery(iqFrom, generateStorageAvatarDocument(iqTo, iqFrom, document.firstChildElement()));
                return QByteArray();
            }
            else if ((xmlns == "http://jabber.org/protocol/bytestreams") && m_serverConfiguration->value("modules").toObject().value("bytestreams").toBool())
            {
                return m_byteStreamManager->byteStreamManagerReply(document, iqFrom);
            }
            else
            {
                return Error::generateError("", "iq", "cancel", "service-unavailable",
                                            Utils::getHost(iqFrom), iqFrom, "", QDomElement());
            }
        }
        else if (firstChildTagName == "ping" && m_serverConfiguration->value("modules").toObject().value("ping").toBool())
        {
            return generatePongReply(Utils::getHost(iqFrom), iqFrom, id);
        }
        else if (((firstChildTagName == "vCard") || (firstChildTagName == "VCARD")) && m_serverConfiguration->value("modules").toObject().value("vcard-temp").toBool())
        {
            return m_vCardManager->vCardManagerReply(document, iqFrom);
        }
        else if ((firstChildTagName == "time") && m_serverConfiguration->value("modules").toObject().value("time").toBool())
        {
            return m_entityTimeManager->entityTimeManagerReply(document, iqFrom);
        }
        else if ((firstChildTagName == "offline") && m_serverConfiguration->value("modules").toObject().value("msgoffline").toBool())
        {
            return m_offlineMessageManager->offlineMessageManagerReply(document, iqFrom);
        }
        else if ((firstChildTagName == "blocklist") && m_serverConfiguration->value("modules").toObject().value("blockingcmd").toBool())
        {
            return m_blockingCmdManager->blockingCommandManagerReply(document, iqFrom);
        }
        else if (firstChildTagName == "start")
        {
            if (firstChild.attribute("xmlns") == "http://jabber.org/protocol/sipub")
            {
                emit sigApplicationRequest(iqTo, document);
            }
            return QByteArray();
        }
        else
        {
            return Error::generateError("", "iq", "cancel", "service-unavailable",
                                        Utils::getHost(iqFrom), iqFrom, "", QDomElement());
        }
    }
    else if (iq.attribute("type") == "result")
    {
        document.firstChildElement().setAttribute("from", iqFrom);
        emit sigApplicationReply(iqTo, document);
        return QByteArray();
    }
    else if (iq.attribute("type") == "error")
    {
        document.firstChildElement().setAttribute("from", iqFrom);
        emit sigApplicationReply(iqTo, document);
        return QByteArray();
    }
    return QByteArray();
}

QDomDocument IqManager::generateStorageAvatarDocument(QString from, QString to, QDomElement avatarElement)
{
    QDomDocument document;
    QDomElement iqElement = document.createElement("iq");
    iqElement.setAttribute("from", from);
    iqElement.setAttribute("to", to);

    QDomElement queryElement = document.createElement("query");
    queryElement.setAttribute("xmlns", "storage:client:avatar");

    queryElement.appendChild(avatarElement);
    iqElement.appendChild(queryElement);
    document.appendChild(iqElement);

    return document;
}

QByteArray IqManager::registerUserReply(QString username, QString password, QString jid, QString id,
                                        QDomElement firstChild, QString iqFrom)
{
    if (username.isEmpty() || password.isEmpty())
    {
        return Error::generateError("", "iq", "modify", "not-acceptable", "", "", id, firstChild);
    }

    // If this user already exist
    if (m_userManager->userExists(jid))
    {
        QString oldPassword = m_userManager->getPassword(jid);
        if (username.isNull() || username.isEmpty())
        {
            return Error::generateError("", "iq", "modify", "bad-request", Utils::getHost(iqFrom), iqFrom, id,
                                        firstChild);
        }

        if (!password.isEmpty() || !password.isNull())
        {
            if (oldPassword != password)
            {
                return DataFormManager::getPasswordChangeForm(Utils::getHost(iqFrom), iqFrom, id).toByteArray();
            }
        }
        return Error::generateError("", "iq", "cancel", "conflict",  "", "", id, firstChild);
    }
    else
    {
        if (m_userManager->createUser(jid, password))
        {
            return generateIQResult("", "", id);
        }
        else
        {
            return Error::generateInternalServerError();
        }
    }
    return QByteArray();
}

QByteArray IqManager::generateRoomAffiliationList(QString from, QString to, QString id, QList<QString> list,
                                                  QString affiliation)
{
    QDomDocument document;
    QDomElement iqElement = document.createElement("iq");
    iqElement.setAttribute("from", from);
    iqElement.setAttribute("to", to);
    iqElement.setAttribute("id", id);
    iqElement.setAttribute("type", "result");

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "http://jabber.org/protocol/muc#admin");
    foreach (QString jid, list)
    {
        QDomElement item = document.createElement("item");
        item.setAttribute("affiliation", affiliation);
        item.setAttribute("jid", jid);

        query.appendChild(item);
    }
    iqElement.appendChild(query);
    document.appendChild(iqElement);

    return document.toByteArray();
}

QByteArray IqManager::generateRoomRoleList(QString from, QString to, QString id, QList<QString> list,
                                                  QString role)
{
    QDomDocument document;
    QDomElement iqElement = document.createElement("iq");
    iqElement.setAttribute("from", from);
    iqElement.setAttribute("to", to);
    iqElement.setAttribute("id", id);
    iqElement.setAttribute("type", "result");

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "http://jabber.org/protocol/muc#admin");
    foreach (QString jid, list)
    {
        QDomElement item = document.createElement("item");
        item.setAttribute("role", role);
        item.setAttribute("jid", jid);

        query.appendChild(item);
    }
    iqElement.appendChild(query);
    document.appendChild(iqElement);

    return document.toByteArray();
}

/*!
 * \brief The IqManager::generateRosterGetResultReply method generate the roster get result from an iq get with the namespace "jabber:iq:roster"
 * \param to
 * \param id
 * \param rosterList
 * \return QByteArray
 */
QByteArray IqManager::generateRosterGetResultReply(QString to, QString id,
                                                   QList<Contact> rosterList)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    iq.setAttribute("id", id);
    iq.setAttribute("to", to);
    iq.setAttribute("type", "result");

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:roster");

    if (rosterList.isEmpty())
    {
        iq.appendChild(query);
        document.appendChild(iq);

        return document.toByteArray();
    }
    else
    {
        foreach (Contact contact, rosterList)
        {
            QDomElement item = document.createElement("item");
            item.setAttribute("jid", contact.getJid());
            item.setAttribute("name", contact.getName());
            item.setAttribute("subscription", contact.getSubscription());

            QSet<QString> groupsList = contact.getGroups();

            if (!groupsList.isEmpty())
            {
                foreach (QString group, contact.getGroups())
                {
                    QDomElement groupNode = document.createElement("group");
                    groupNode.appendChild(document.createTextNode(group));
                    item.appendChild(groupNode);
                }
            }
            query.appendChild(item);
        }
        iq.appendChild(query);
        document.appendChild(iq);

        // Request Acknowledgment of receipt
        sigSendReceiptRequest(to, document.toByteArray());
        return document.toByteArray();
    }
}

/*!
 * \brief The IqManager::generateIqSessionReply method generate an iq response from an iq
 * \param id
 * \param from
 * \return QByteArray
 */
QByteArray IqManager::generateIqSessionReply(QString id, QString from)
{
    QDomDocument document;
    QDomElement iqResult = document.createElement("iq");

    iqResult.setAttribute("type", "result");
    iqResult.setAttribute("id", id);
    iqResult.setAttribute("from", from);

    document.appendChild(iqResult);
    return document.toByteArray();
}

/*!
 * \brief The IqManager::generateIQResult method generate an iq result as result from an iq request
 * \param to
 * \param id
 * \return QByteArray
 */
QByteArray IqManager::generateIQResult(QString from, QString to, QString id)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    if (!from.isEmpty())
    {
        iq.setAttribute("from", from);
    }

    if (!to.isEmpty())
    {
        iq.setAttribute("to", to);
    }
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    document.appendChild(iq);

    // Request Acknowledgment of receipt
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

/*!
 * \brief The IqManager::generateRegistrationFieldsReply method generate the registration fields to an user for account registration
 * \param id
 * \return QByteArray
 */
QByteArray IqManager::generateRegistrationFieldsReply(QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:register");

    query.appendChild(document.createElement("username"));
    query.appendChild(document.createElement("password"));

    iq.appendChild(query);
    document.appendChild(iq);

    return document.toByteArray();
}

/*!
 * \brief The IqManager::generateAlreadyRegisterReply method return iq result when an XMPP client want to register an account which is already created
 * \param username
 * \param password
 * \param id
 * \return QByteArray
 */
QByteArray IqManager::generateAlreadyRegisterReply(QString username, QString password, QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:register");

    QDomElement usernameNode = document.createElement("username");
    usernameNode.appendChild(document.createTextNode(username));

    QDomElement passwordNode = document.createElement("password");
    passwordNode.appendChild(document.createTextNode(password));

    query.appendChild(document.createElement("register"));
    query.appendChild(usernameNode);
    query.appendChild(passwordNode);
    iq.appendChild(query);
    document.appendChild(iq);

    return document.toByteArray();
}

/*!
 * \brief The IqManager::generatePongReply method generate pong reply to an iq ping request from an XMPP client
 * \param from
 * \param to
 * \param id
 * \return QByteArray
 */
QByteArray IqManager::generatePongReply(QString from, QString to, QString id)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    iq.setAttribute("from", from);
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");
    document.appendChild(iq);

    // Request Acknowledgment of receipt
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}
