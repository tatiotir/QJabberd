#include "MessageManager.h"

/*!
 * \brief The MessageManager::MessageManager constructor of the message manager
 * \param userManager
 * \param privacyListManager
 */
MessageManager::MessageManager(QJsonObject *serverConfiguration, UserManager *userManager, PrivacyListManager *privacyListManager,
                               MucManager *mucManager, BlockingCommandManager *blockingCmdManager)
{
    m_serverConfiguration = serverConfiguration;
    m_userManager = userManager;
    m_privacyListManager = privacyListManager;
    m_mucManager = mucManager;
    m_blockingCmdManager = blockingCmdManager;
}

/*!
 * \brief The MessageManager::parseMessage method parse a message request from XMPP client
 * \param messageXML
 * \param messageFrom
 * \return QByteArray
 */
QByteArray MessageManager::parseMessage(QDomDocument document, QString messageFrom)
{
    QDomElement messageNode = document.firstChildElement();
    QString from = messageNode.attribute("from", messageFrom);
    QString to = messageNode.attribute("to");
    QString type = messageNode.attribute("type");
    QString id = messageNode.attribute("id", Utils::generateId());
    QString xmlLang = messageNode.attribute("xml:lang");

    if (type != "groupchat")
    {
        QByteArray privacyListError = m_privacyListManager->isBlocked(from, to, "message");
        if (!privacyListError.isEmpty())
            return privacyListError;

        QByteArray privacyListError1 = m_privacyListManager->isBlocked(to, from, "message");
        if (!privacyListError1.isEmpty())
            return privacyListError1;

        QByteArray blockListError = m_blockingCmdManager->isBlocked(from, to, "message");
        if (!blockListError.isEmpty())
            return blockListError;
    }

    /*if (!m_userManager->userExists(Utils::getBareJid(to)))
    {
        return Error::generateServiceUnavailableError("iq", from, to, Utils::generateId());
    }*/

    if (messageNode.firstChildElement().tagName() == "env:Envelope")
    {
        emit sigApplicationRequest(to, document);
        return QByteArray();
    }
    else if ((messageNode.firstChildElement().tagName() == "data") && m_serverConfiguration->value("modules").toObject().value("ibb").toBool())
    {
        emit sigApplicationRequest(to, document);
        return QByteArray();
    }
    else if (messageNode.elementsByTagName("x").item(0).toElement().attribute("xmlns") == "jabber:x:obb")
    {
        emit sigApplicationRequest(to, document);
        return QByteArray();
    }

    if ((type == "chat") || (type == "normal"))
    {
        if (m_mucManager->chatRoomExist(Utils::getBareJid(to)))
        {
            QString roomName = Utils::getBareJid(to);
            QString receiverJid = m_mucManager->getOccupantJid(roomName, to);
            QString senderMucJid = m_mucManager->getOccupantMucJid(roomName, from);

            if (receiverJid.isEmpty())
                return Error::generateError(Utils::getBareJid(to), "message", "cancel", "item-not-found",
                                            to, from, id, messageNode.firstChildElement());

            if (senderMucJid.isEmpty())
                return Error::generateError(roomName, "message", "modify", "not-acceptable", to, from,
                                            Utils::generateId(),
                                            messageNode.firstChildElement());

            messageNode.setAttribute("from", senderMucJid);
            messageNode.setAttribute("to", receiverJid);

            emit sigGroupchatMessage(receiverJid, document);
            return QByteArray();
        }
        // One to one chat
        messageNode.setAttribute("from", from);
        emit sigNewChatMessage(to, document);
    }
    else if (type == "groupchat")
    {
        if (to.contains("/"))
        {
            return Error::generateError(Utils::getBareJid(to), "message", "modify", "bad-request",
                                        to, from, id, messageNode.firstChildElement());
        }

        QString roomName = to;
        QString mucJid = m_mucManager->getOccupantMucJid(roomName, from);
        bool hasVoice = m_mucManager->hasVoice(roomName, from);
        messageNode.setAttribute("from", mucJid);

        // Converting one-to-one chat to groupchat
        if (!messageNode.elementsByTagName("delay").isEmpty())
        {
            QDomElement delayElement = messageNode.elementsByTagName("delay").item(0).toElement();
            delayElement.setAttribute("from", to);
            m_mucManager->saveMucMessage(roomName, document.toByteArray(), delayElement.attribute("stamp"));
        }

        if ((m_mucManager->getRoomTypes(roomName).contains("moderated")) && !hasVoice)
        {
            return Error::generateError(roomName, "message", "auth", "forbidden", to, from,
                                        Utils::generateId(),
                                        messageNode.firstChildElement());
        }

        // Not an occupant of the room
        if (mucJid.isEmpty())
        {
            return Error::generateError(roomName, "message", "modify", "not-acceptable", to, from,
                                        Utils::generateId(),
                                        messageNode.firstChildElement());
        }

        if (messageNode.firstChildElement().tagName() == "subject")
        {
            if (!m_mucManager->canChangeRoomSubject(roomName))
            {
                return Error::generateError(roomName, "message", "auth", "forbidden", to, from,
                                            Utils::generateId(),
                                            messageNode.firstChildElement());
            }
            else
            {
                QString subject = messageNode.firstChildElement().text();
                m_mucManager->changeRoomSubject(roomName, subject);

                QList<Occupant> occupantsList = m_mucManager->getOccupants(roomName);
                messageNode.setAttribute("from", m_mucManager->getOccupantMucJid(roomName, from));
                foreach (Occupant occupant, occupantsList)
                {
                    messageNode.setAttribute("to", occupant.jid());
                    emit sigGroupchatMessage(occupant.jid(), document);
                }
            }
        }
        else
        {
            if (hasVoice)
            {
                QList<Occupant> occupantList = m_mucManager->getOccupants(roomName);
                foreach (Occupant occupant, occupantList)
                {
                    messageNode.setAttribute("to", occupant.jid());
                    emit sigGroupchatMessage(occupant.jid(), document);
                }

                // save the message
                QDateTime dateTime(QDateTime::currentDateTime());
                dateTime.setTimeSpec(Qt::UTC);

                QString stamp = dateTime.toString("yyyy-MM-ddThh:mm:ss.zzz") + "Z";
                QDomElement delayElement = document.createElement("delay");
                delayElement.setAttribute("xmlns", "urn:xmpp:delay");
                delayElement.setAttribute("from", roomName);
                delayElement.setAttribute("stamp", stamp);

                messageNode.appendChild(delayElement);
                m_mucManager->saveMucMessage(roomName, document.toByteArray(), stamp);
            }
        }
    }
    else if (type.isEmpty())
    {
        QDomElement xElement = messageNode.firstChildElement();
        // Direce MUC Invitation
        if (xElement.attribute("xmlns") == "jabber:iq:conference")
        {
            emit sigDirectMucInvitation(to, document);
        }
        else if (xElement.attribute("xmlns") == "http://jabber.org/protocol/muc#user")
        {
            QString roomName = to;

            if (xElement.firstChildElement().tagName() == "invite")
            {
                if (m_mucManager->getRoomTypes(roomName).contains("membersonly"))
                {
                    // Does not has the good privileges
                    if (m_mucManager->getOccupantAffiliation(roomName, from) != "admin")
                    {
                        return Error::generateError(roomName, "message", "auth", "forbidden", to, from,
                                                    Utils::generateId(),
                                                    messageNode.firstChildElement());
                    }
                }

                QString occupantMucJid = m_mucManager->getOccupantMucJid(roomName, from);
                // Non existent jid in the room
                if (occupantMucJid.isEmpty())
                {
                    return Error::generateError(to, "message", "cancel", "item-not-found",
                                                to, from, id, messageNode.firstChildElement());
                }

                QDomNodeList inviteNodeList = messageNode.elementsByTagName("invite");
                for (int i = 0; i < inviteNodeList.count(); ++i)
                {
                    QDomElement inviteElement = inviteNodeList.item(i).toElement();

                    QDomDocument invitationDocument;

                    QDomElement xEle = invitationDocument.createElement("x");
                    xEle.setAttribute("xmlns", "http://jabber.org/protocol/muc#user");

                    QDomElement messageElement = invitationDocument.createElement("message");
                    messageElement.setAttribute("from", roomName);
                    messageElement.setAttribute("to", inviteElement.attribute("to"));

                    inviteElement.removeAttribute("to");
                    inviteElement.setAttribute("from", from);

                    xEle.appendChild(inviteElement);
                    messageElement.appendChild(xEle);
                    invitationDocument.appendChild(messageElement);

                    if (m_mucManager->isPasswordProtectedRoom(roomName))
                    {
                        QDomElement passwordElement = document.createElement("password");
                        passwordElement.appendChild(document.createTextNode(m_mucManager->getRoomPassword(to)));
                        xEle.appendChild(passwordElement);
                    }

                    qDebug() << "invitation : " << invitationDocument.toByteArray();
                    // Send mediated invitation
                    emit sigGroupchatMessage(messageElement.attribute("to"), invitationDocument);
                }
            }
            else if (xElement.firstChildElement().tagName() == "decline")
            {
                QDomElement declineElement = xElement.firstChildElement();
                messageNode.setAttribute("from", roomName);
                messageNode.setAttribute("to", declineElement.attribute("to"));

                declineElement.removeAttribute("to");
                declineElement.setAttribute("from", Utils::getBareJid(from));
                emit sigGroupchatMessage(messageNode.attribute("to"), document);
            }
        }
        else if (xElement.attribute("xmlns") == "jabber:iq:data")
        {
            QMap<QString, QVariant> dataFormValue = DataFormManager::parseDataForm(xElement);
            if (dataFormValue.value("FORM_TYPE") == "http://jabber.org/protocol/muc#request")
            {
                QString formType = xElement.attribute("type");
                if (formType == "form")
                {
                    QList<QString> moderatorJidList = m_mucManager->getRoomModeratorsJid(to);
                    QString requesterNickname = Utils::getResource(m_mucManager->getOccupantMucJid(to, from));
                    QString requesterRole = dataFormValue.value("muc#role").toString();

                    QDomDocument document = DataFormManager::getRoomVoiceRequestForm(to, from, id, requesterRole, from, requesterNickname);
                    foreach (QString moderatorJid, moderatorJidList)
                    {
                        document.firstChildElement().setAttribute("to", moderatorJid);
                        emit sigGroupchatMessage(moderatorJid, document);
                    }
                }
                else if (formType == "submit")
                {
                    if (dataFormValue.value("muc#request_allow").toBool())
                    {
                        QString mucJid = to + "/" + dataFormValue.value("muc#roomnick").toString();
                        QString role = dataFormValue.value("muc#role").toString();
                        m_mucManager->changeRole(to, mucJid, role);

                        QList<Occupant> occupantList = m_mucManager->getOccupants(to);
                        QDomDocument document = Utils::generatePresence("", mucJid, mucJid,
                                                                        Utils::generateId(),
                                                                        m_mucManager->getOccupantAffiliationFromMucJid(to, mucJid), role, "",
                                                                        dataFormValue.value("muc#roomnick").toString(), QList<int>(), "", "");
                        foreach (Occupant occupant, occupantList)
                        {
                            document.firstChildElement().setAttribute("to", occupant.jid());
                            emit sigMucPresenceBroadCast(occupant.jid(), document);
                        }

                        QDomDocument doc;
                        QDomElement iq = doc.createElement("iq");
                        iq.setAttribute("from", to);
                        iq.setAttribute("id", id);
                        iq.setAttribute("to", from);
                        document.appendChild(iq);

                        return document.toByteArray();
                    }
                }
            }
        }
    }
    return QByteArray();
}
