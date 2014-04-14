#include "IqManager.h"

IQManager::IQManager(QMap<QString, QVariant> *serverConfigMap, UserManager *userManager, PrivacyListManager *privacyListManager,
                     RosterManager *rosterManager, VCardManager *vcardManager,
                     LastActivityManager *lastActivityManager, EntityTimeManager *entityTimeManager,
                     PrivateStorageManager *privateStorageManager,
                     ServiceDiscoveryManager *serviceDiscoveryManager, OfflineMessageManager *offlineMessageManager)
{
    m_serverConfigMap = serverConfigMap;
    m_userManager = userManager;
    m_privacyListManager = privacyListManager;
    m_vCardManager = vcardManager;
    m_lastActivityManager = lastActivityManager;
    m_entityTimeManager = entityTimeManager;
    m_rosterManager = rosterManager;
    m_privateStorageManager = privateStorageManager;
    m_serviceDiscoveryManager = serviceDiscoveryManager;
    m_offlineMessageManager = offlineMessageManager;
}

QByteArray IQManager::parseIQ(QByteArray iqXML, QString from, QString host)
{
    QDomDocument document;
    document.setContent(iqXML);

    QDomElement iq = document.firstChild().toElement();

    QString iqFrom = iq.attribute("from", from);
    QString id = iq.attribute("id", Utils::generateId());

//    if (!m_userManager->userExists(Utils::getBareJid(iqTo)))
//    {
//        return Error::generateServiceUnavailableError("iq", iqFrom, iqTo, id);
//    }

    if (iq.attribute("type") == "set")
    {
        QDomElement firstChild = iq.firstChild().toElement();
        QString firstChildTagName = firstChild.tagName();

        if (firstChildTagName == "session")
        {
            return generateIqSessionReply(id);
        }
        else if (firstChildTagName == "query")
        {
            QString xmlns = firstChild.attribute("xmlns");

            if ((xmlns == "jabber:iq:roster") && m_serverConfigMap->value("modules").toMap().value("roster").toBool())
            {
                // We check if there are errors.
                QDomNodeList groupNodes = firstChild.firstChild().toElement().elementsByTagName("group");
                for (int i = 0; i < groupNodes.count(); ++i)
                {
                    // if one of the group node text is empty
                    QDomElement groupElement = groupNodes.item(i).toElement();
                    if ((groupElement.text().isEmpty()) || (groupElement.text().count() > 100))
                    {
                        return Error::generateError("iq", "modify", "not-acceptable", iqFrom, "", id, QDomElement());
                    }
                }

                if (firstChild.elementsByTagName("item").count() > 1)
                {
                    return Error::generateError("iq", "modify", "bad-request", iqFrom, "", id, QDomElement());
                }

                // the name attribute is too long
                if (firstChild.firstChild().toElement().attribute("name").count() > 100)
                {
                    return Error::generateError("iq", "modify", "not-acceptable", iqFrom, "", id, QDomElement());
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
                        return Error::generateError("iq", "cancel", "item-not-found", iqFrom, "", id, QDomElement());
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
                        return generateIQResult(iqFrom, id);
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

                            return generateIQResult(iqFrom, id);
                        }
                        else
                        {
                            return QByteArray("Error");
                            // return internal server error
                        }
                    }
                }
            }
            else if (xmlns == "jabber:iq:register" && m_serverConfigMap->value("modules").toMap().value("register").toBool())
            {
                // Data form
                if (firstChild.firstChild().toElement().tagName() == "x")
                {
                    QMultiHash<QString, QString> dataFormValue = DataForm::parseDataForm(
                                firstChild.firstChild().toElement());

                    QString formType = dataFormValue.value("FORM_TYPE");
                    if (formType == "jabber:iq:register")
                    {

                    }
                    else if (formType == "jabber:iq:register:changepassword")
                    {

                    }
                }
                else if (firstChild.firstChild().toElement().tagName() == "remove")
                {
                    if (firstChild.elementsByTagName("remove").count() > 1)
                    {
                        return Error::generateError("iq", "modify", "bad-request", Utils::getHost(iqFrom), "", id, QDomElement());
                    }
                    // We verify others errors

                    if (m_userManager->deleteUser(Utils::getBareJid(from)))
                    {
                        return generateIQResult(iqFrom, id);
                    }
                    else
                    {
                        //
                    }
                }
                else
                {
                    QDomElement usernameNode = firstChild.elementsByTagName("username").item(0).toElement();
                    QDomElement passwordNode = firstChild.elementsByTagName("password").item(0).toElement();

                    QString username = usernameNode.text();
                    QString password = passwordNode.text();
                    QString jid = username + "@" + host;

                    if (usernameNode.isNull() || passwordNode.isNull())
                    {
                        return Error::generateError("iq", "modify", "not-acceptable", "", "", id, firstChild);
                    }

                    // If this user already exist
                    if (m_userManager->userExists(jid))
                    {
                        QString oldPassword = m_userManager->getPassword(jid);
                        if (username.isNull() || username.isEmpty())
                        {
                            return Error::generateError("iq", "modify", "bad-request", Utils::getHost(iqFrom), iqFrom, id,
                                                        firstChild);
                        }

                        if (!password.isEmpty() || !password.isNull())
                        {
                            if (oldPassword != password)
                            {
                                return DataForm::getPasswordChangeForm(Utils::getHost(iqFrom),
                                                                       iqFrom, id);
                            }
                        }
                        return Error::generateError("iq", "cancel", "conflict",  "", "", id, firstChild);
                    }
                    else
                    {
                        bool ok = m_userManager->createUser(jid, password);

                        if (ok)
                        {
                            return generateIQResult("", id);
                        }
                        else
                        {
                            // return internal server error
                        }
                    }
                }
            }
//            else if (xmlns == "jabber:iq:privacy")
//            {
//                return m_privacyListManager->privacyListReply(iqXML, iqFrom);
//            }
            else if (xmlns == "jabber:iq:private" && m_serverConfigMap->value("modules").toMap().value("private").toBool())
            {
                return m_privateStorageManager->privateStorageManagerReply(iqXML, iqFrom);
            }
            else
            {
                return Error::generateError("iq", "cancel", "service-unavalaible",
                                                              Utils::getHost(iqFrom), iqFrom, "", QDomElement());
            }
        }
        else if (((firstChildTagName == "vCard") || (firstChildTagName == "VCARD")) && m_serverConfigMap->value("modules").toMap().value("vcard-temp").toBool())
        {
            return m_vCardManager->vCardManagerReply(iqXML, iqFrom);
        }
        else if (firstChildTagName == "offline" && m_serverConfigMap->value("modules").toMap().value("msgoffline").toBool())
        {
            return m_offlineMessageManager->offlineMessageManagerReply(iqXML, iqFrom);
        }
        else
        {
            return Error::generateError("iq", "cancel", "service-unavalaible",
                                               Utils::getHost(iqFrom), iqFrom, "", QDomElement());
        }
    }
    else if (iq.attribute("type") == "get")
    {
        QDomElement firstChild = iq.firstChild().toElement();
        QString firstChildTagName = firstChild.tagName();

        if (firstChildTagName == "query")
        {
            QString xmlns = firstChild.attribute("xmlns");
            if ((xmlns == "jabber:iq:roster") && m_serverConfigMap->value("modules").toMap().value("roster").toBool())
            {
                if (firstChild.text().isEmpty())
                {
                    QList<Contact> contactList = m_rosterManager->getContactsList(Utils::getBareJid(iqFrom));
                    return generateRosterGetResultReply(iqFrom, id, contactList);
                }
            }
            else if ((xmlns == "jabber:iq:register") && m_serverConfigMap->value("modules").toMap().value("register").toBool())
            {
                return generateRegistrationFieldsReply(id);
            }
            else if (xmlns.contains("http://jabber.org/protocol/disco") && m_serverConfigMap->value("modules").toMap().value("disco").toBool())
            {
                if (xmlns == "http://jabber.org/protocol/disco#info")
                {
                    return m_serviceDiscoveryManager->serviceDiscoveryManagerReply(iqXML, iqFrom);
                }
                else if (xmlns == "http://jabber.org/protocol/disco#items")
                {
                    return m_serviceDiscoveryManager->serviceDiscoveryManagerReply(iqXML, iqFrom);
                }
            }
//            else if (xmlns == "jabber:iq:privacy")
//            {
//                return m_privacyListManager->privacyListReply(iqXML, iqFrom);
//            }
            else if (xmlns == "jabber:iq:last")
            {
                return m_lastActivityManager->lastActivityReply(iqXML, iqFrom);
            }
            else if ((xmlns == "jabber:iq:private") && m_serverConfigMap->value("modules").toMap().value("private").toBool())
            {
                return m_privateStorageManager->privateStorageManagerReply(iqXML, iqFrom);
            }
            else
            {
                return Error::generateError("iq", "cancel", "service-unavalaible",
                                                              Utils::getHost(iqFrom), iqFrom, "", QDomElement());
            }
        }
        else if (firstChildTagName == "ping" && m_serverConfigMap->value("modules").toMap().value("ping").toBool())
        {
            return generatePongReply(Utils::getHost(iqFrom), iqFrom, id);
        }
        else if (((firstChildTagName == "vCard") || (firstChildTagName == "VCARD")) && m_serverConfigMap->value("modules").toMap().value("vcard-temp").toBool())
        {
            return m_vCardManager->vCardManagerReply(iqXML, iqFrom);
        }
        else if ((firstChildTagName == "time") && m_serverConfigMap->value("modules").toMap().value("time").toBool())
        {
            return m_entityTimeManager->entityTimeManagerReply(iqXML, iqFrom);
        }
        else if ((firstChildTagName == "offline") && m_serverConfigMap->value("modules").toMap().value("msgoffline").toBool())
        {
            return m_offlineMessageManager->offlineMessageManagerReply(iqXML, iqFrom);
        }
        else
        {
            return Error::generateError("iq", "cancel", "service-unavalaible",
                                                          Utils::getHost(iqFrom), iqFrom, "", QDomElement());
        }
    }
    return QByteArray();
}

QByteArray IQManager::generateRosterGetResultReply(QString to, QString id,
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

QByteArray IQManager::generateIqSessionReply(QString id)
{
    QDomDocument document;
    QDomElement iqResult = document.createElement("iq");

    iqResult.setAttribute("type", QString("result"));
    iqResult.setAttribute("id", id);
    iqResult.setAttribute("from", "localhost");

    document.appendChild(iqResult);

    return document.toByteArray();
}

QByteArray IQManager::generateIQResult(QString to, QString id)
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

    // Request Acknowledgment of receipt
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

QByteArray IQManager::generateRegistrationFieldsReply(QString id)
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

QByteArray IQManager::generateAlreadyRegisterReply(QString username, QString password, QString id)
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

QByteArray IQManager::generatePongReply(QString from, QString to, QString id)
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
