#include "StreamManager.h"

StreamManager::StreamManager(QObject *parent, StorageManager *storageManager, UserManager *userManager, RosterManager *rosterManager,
                             LastActivityManager *lastActivityManager) :
    QThread(parent)
{
    m_userMap = new QMultiHash<QString, User* >();
    m_notNegotiatedStream = new QMultiHash<QString, Stream* >();
    m_storageManager = storageManager;
    m_userManager = userManager;
    m_rosterManager = rosterManager;
    m_lastActivityManager = lastActivityManager;

    m_serverTime.start();
}

void StreamManager::newConnection(Connection *connection, IQManager *iqManager,
                                  PresenceManager *presenceManager, MessageManager *messageManager,
                                  RosterManager *rosterManager,
                                  StreamNegotiationManager *streamNegotiationManager)
{
    QString streamId = Utils::generateId();
    Stream *stream = new Stream(this, streamId, connection, iqManager, presenceManager, messageManager,
                                rosterManager, streamNegotiationManager);

    m_notNegotiatedStream->insert(streamId, stream);

    connect(stream, SIGNAL(sigBindFeatureNegotiated(QString,Connection*)), this,
            SLOT(saveStream(QString,Connection*)));

    connect(stream, SIGNAL(sigOfflineUser(QString)), this, SLOT(offlineUser(QString)));

    connect(stream, SIGNAL(sigBindFeatureNegotiated(QString,Connection*)), this,
            SLOT(saveStream(QString,Connection*)));

    connect(stream, SIGNAL(sigPresenceBroadCast(QString,QByteArray)), this,
            SLOT(presenceBroadCast(QString,QByteArray)));

    connect(stream, SIGNAL(sigPresenceUnavailableBroadCast(QString,QString)), this,
            SLOT(presenceUnavailableBroadCast(QString,QString)));

    connect(stream, SIGNAL(sigEnableStreamManagement(QString,QString)), this,
            SLOT(enableStreamManagement(QString,QString)));

    connect(stream, SIGNAL(sigInboundStanzaReceived(QString)), this, SLOT(inboundStanzaReceived(QString)));
    connect(stream, SIGNAL(sigQueryInboundStanzaReceived(QString)), this,
            SLOT(queryInboundStanzaReceived(QString)));

    connect(stream, SIGNAL(sigAcknowledgeReceiptServerStanza(QString,int)), this,
            SLOT(acknowledgeReceiptServerStanza(QString,int)));

    connect(stream, SIGNAL(sigResumeStream(Connection*,QString,int)), this,
            SLOT(resumeStream(Connection*,QString,int)));

    stream->start();
}

void StreamManager::resourceBindingReply(QString streamId, QString fullJid, QString id)
{
    QString jid = fullJid;
    QDomDocument document;
    QDomElement iqResult = document.createElement("iq");

    iqResult.setAttribute("type", QString("result"));
    iqResult.setAttribute("id", id);

    QDomElement bind = document.createElement("bind");
    bind.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-bind");

    QDomElement jidNode = document.createElement("jid");

    if (m_userMap->keys().contains(fullJid))
        jid += Utils::generateId();

    jidNode.appendChild(document.createTextNode(jid));

    bind.appendChild(jidNode);
    iqResult.appendChild(bind);
    document.appendChild(iqResult);

    m_notNegotiatedStream->value(streamId)->streamReply(document.toByteArray());
    m_notNegotiatedStream->value(streamId)->setHost(Utils::getHost(fullJid));
    m_notNegotiatedStream->value(streamId)->bindFeatureNegotiated(jid);

    // Delete this stream in non negotiated stream map
    m_notNegotiatedStream->remove(streamId);
}

void StreamManager::streamNegotiationError(QString streamId)
{
    // Send the close xml tag
    m_notNegotiatedStream->value(streamId)->streamReply(QByteArray("</stream:stream>"));

    // Close stream's thread
    m_notNegotiatedStream->value(streamId)->exit(0);

    // Delete the stream
    m_notNegotiatedStream->remove(streamId);
}

void StreamManager::nonSaslAuthentification(QString streamId, QString fullJid, QString id)
{
    if (m_userMap->keys().contains(fullJid))
    {
        m_notNegotiatedStream->value(streamId)->streamReply(Error::generateError("iq", "cancel", "conflict", "", "", id, QDomElement())
                                                            + QByteArray("</stream:stream>"));
    }
    else
    {
        QDomDocument document;
        QDomElement iq = document.createElement("iq");

        iq.setAttribute("type", "result");
        iq.setAttribute("id", id);
        document.appendChild(iq);
        m_notNegotiatedStream->value(streamId)->streamReply(document.toByteArray());
        m_notNegotiatedStream->value(streamId)->setHost(Utils::getHost(fullJid));
        m_notNegotiatedStream->value(streamId)->bindFeatureNegotiated(fullJid);
    }

    // Delete this stream in non negotiated stream map
    m_notNegotiatedStream->remove(streamId);
}

void StreamManager::enableStreamManagement(QString fullJid, QString smId)
{
    qDebug() << "enable presence management : from = " << fullJid << " smId = " << smId;
    m_userMap->value(fullJid)->setSmId(smId);
}

void StreamManager::clientServiceDiscoveryQuery(QString to, QByteArray request)
{
    m_userMap->value(to)->getConnection()->write(request);
    m_userMap->value(to)->getConnection()->flush();
}

void StreamManager::currentPresence(QString fullJid, QByteArray presenceData)
{
    qDebug() << "presence information : " << " jid = " << fullJid << " information = " << presenceData;
    m_userMap->value(fullJid)->setCurrentPresence(presenceData);

    QDomDocument document;
    document.setContent(presenceData);
    m_lastActivityManager->setLastStatus(Utils::getBareJid(fullJid),
                                 document.documentElement().elementsByTagName("status").item(0).toElement().text());

    if (!m_userMap->value(fullJid)->getSmId().isEmpty())
    {
        saveStreamPresenceStanza(m_userMap->value(fullJid)->getSmId(), presenceData);
    }
}

void StreamManager::offlineUser(QString fullJid)
{
    // Save the logout time
    m_lastActivityManager->setLastLogoutTime(Utils::getBareJid(fullJid), QDateTime::currentDateTime().toString("dd/MM/yyyy"));

    // Save the last status
    QDomDocument document;
    document.setContent(m_userMap->value(fullJid)->getCurrentPresence());

    m_lastActivityManager->setLastStatus(Utils::getBareJid(fullJid),
                                 document.documentElement().elementsByTagName("status").item(0).toElement().text());
    m_userMap->remove(fullJid);
}

void StreamManager::saveStream(QString fullJid, Connection *connection)
{
    m_userMap->insert(fullJid, new User(connection, "", 0, 0, QByteArray()));
    sendUndeliveredPresence(fullJid);
    //sendOfflineMessage(fullJid);
}

void StreamManager::sendOfflineMessage(QString to)
{
    QMultiHash<QString, QByteArray> offlineMessages = getAllOfflineMessage(Utils::getBareJid(to));

    QList<QString> keyList = offlineMessages.keys();
    foreach (QString key, keyList)
    {
        QDomDocument document;
        document.setContent(offlineMessages.value(key));

        QDomElement delayElement = document.createElement("delay");
        delayElement.setAttribute("xmlns", "urn:xmpp:delay");
        delayElement.setAttribute("from", Utils::getHost(to));
        delayElement.setAttribute("stamp", key);
        document.documentElement().appendChild(delayElement);

        sendMessage(to, document.toByteArray());
    }
}

void StreamManager::sendUndeliveredPresence(QString to)
{
    QList<QVariant> presenceSubscriptionList = getOfflinePresenceSubscription(Utils::getBareJid(to));

    qDebug() << "Undelevered presence to : " << Utils::getBareJid(to) << " Presence : " << presenceSubscriptionList;
    if (!presenceSubscriptionList.isEmpty())
    {
        foreach (QVariant presenceSubscription, presenceSubscriptionList)
        {
            presenceBroadCast(to, presenceSubscription.toByteArray());
        }
    }

//    // Request acknowledgment receipt
//    if (m_userMap->value(to)->getSmId().isEmpty())
//    {
//        int count = listSubscribe.count() + listSubscribed.count() + listUnsubscribe.count()
//                + listUnsubscribed.count();
//        for (int i = 0; i < count; ++i)
//        {
//            sendReceiptRequest(to);
//        }
//    }
}

void StreamManager::rosterPush(QString to, QByteArray data)
{
    QDomDocument document;
    document.setContent(data);
    QDomElement iq = document.documentElement();

    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());
    QStringList toResourceList = keyList.filter(to, Qt::CaseInsensitive);
    foreach (QString toResource, toResourceList)
    {
        iq.setAttribute("to", toResource);
        m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
        m_userMap->value(toResource)->getConnection()->flush();

        // Request acknowledgment of receipt
        if (!m_userMap->value(toResource)->getSmId().isEmpty())
        {
            sendReceiptRequest(toResource, document.toByteArray());
        }
        qDebug() << "I send roster push to : " << toResource <<  " : push : " << document.toByteArray();
    }

}

void StreamManager::presencePriority(QString fullJid, int priority)
{
    qDebug() << "Presence priority : " << priority << " From : " << fullJid;
    m_userMap->value(fullJid)->setPresencePriority(priority);

    if (!m_userMap->value(fullJid)->getSmId().isEmpty())
    {
        saveStreamPresencePriority(m_userMap->value(fullJid)->getSmId(), priority);
    }
}

void StreamManager::presenceBroadCast(QString to, QByteArray data)
{
    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());
    QStringList toResourceList = keyList.filter(to, Qt::CaseInsensitive);

    QDomDocument document;
    document.setContent(data);

    QDomElement presenceElement = document.documentElement();
    QString presenceType = presenceElement.attribute("type");

    if (!toResourceList.isEmpty())
    {
        QString from = presenceElement.attribute("from");
        QString to = presenceElement.attribute("to");

        if (presenceType.isEmpty() || (presenceType == "unavailable"))
        {
            qDebug() << "presence broadcast data : " << document.toByteArray();
            foreach (QString toResource, toResourceList)
            {
                m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
                m_userMap->value(toResource)->getConnection()->flush();

                // Request acknowledgment of receipt
                if (!m_userMap->value(toResource)->getSmId().isEmpty())
                {
                    sendReceiptRequest(toResource, document.toByteArray());
                }
            }

            QStringList fromResourceList = keyList.filter(Utils::getBareJid(from), Qt::CaseInsensitive);
            if (!fromResourceList.isEmpty())
            {
                foreach (QString fromResource, fromResourceList)
                {
                    presenceElement.setAttribute("to", fromResource);
                    m_userMap->value(fromResource)->getConnection()->write(document.toByteArray());
                    m_userMap->value(fromResource)->getConnection()->flush();

                    // Request acknowledgment of receipt
                    if (!m_userMap->value(fromResource)->getSmId().isEmpty())
                    {
                        sendReceiptRequest(fromResource, document.toByteArray());
                    }
                }
            }
        }
        else if (presenceType == "subscribe")
        {
            qDebug() << "presence subscribe : " << m_rosterManager->getContact(to, from).getSubscription()
                     << " " << m_rosterManager->getContact(to, from).getApproved();
            // Chech if the contact who must receive this presence subscribe has pre-approved the presence
            // subscribe from this user
            if (m_rosterManager->contactExists(to, from))
            {
                if ((m_rosterManager->getContact(to, from).getApproved() ||
                     (m_rosterManager->getContact(to, from).getSubscription() == "to")))
                {
                    QByteArray presenceData = Utils::generatePresence("subscribed", to, from, Utils::generateId(),
                                                                      "", "", "");
                    qDebug() << "je suis ici oh oui : " << presenceData;
                    QStringList fromResourceList = keyList.filter(from, Qt::CaseInsensitive);
                    if (!fromResourceList.isEmpty())
                    {
                        foreach (QString fromResource, fromResourceList)
                        {
                            m_userMap->value(fromResource)->getConnection()->write(presenceData);
                            m_userMap->value(fromResource)->getConnection()->flush();

                            // Request acknowledgment of receipt
                            if (!m_userMap->value(fromResource)->getSmId().isEmpty())
                            {
                                sendReceiptRequest(fromResource, presenceData);
                            }
                        }
                    }
                    else
                    {
                        // Save the subscribe presence for later deliver
                        saveOfflinePresenceSubscription(to, from, presenceData, "subscribed");
                    }

                    Contact userContact = m_rosterManager->getContact(from, to);

                    m_rosterManager->updateAskAttributeToContact(from, to, "");
                    m_rosterManager->updateSubscriptionToContact(from, to, "to");

                    userContact = m_rosterManager->getContact(from, to);

                    // Send roster push
                    rosterPush(from, Utils::generateRosterPush(from, Utils::generateId(),
                                                             userContact.getJid(),
                                                             userContact.getName(),
                                                             userContact.getSubscription(),
                                                             userContact.getAsk(),
                                                             userContact.getApproved(),
                                                             userContact.getGroups()));

                    m_rosterManager->updateAskAttributeToContact(to, from, "");
                    m_rosterManager->updateSubscriptionToContact(to, from, "from");

                    userContact = m_rosterManager->getContact(to, from);

                    // Send roster push
                    rosterPush(to, Utils::generateRosterPush(to, Utils::generateId(),
                                                             userContact.getJid(),
                                                             userContact.getName(),
                                                             userContact.getSubscription(),
                                                             userContact.getAsk(),
                                                             userContact.getApproved(),
                                                             userContact.getGroups()));

                    // Send presence to the contact to whom i subscribed
                    presenceBroadCastFromContact(from, to);
                }
                else
                {
                    qDebug() << "presence broadcast data : " << document.toByteArray();
                    foreach (QString toResource, toResourceList)
                    {
                        m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
                        m_userMap->value(toResource)->getConnection()->flush();

                        // Request acknowledgment of receipt
                        if (!m_userMap->value(toResource)->getSmId().isEmpty())
                        {
                            sendReceiptRequest(toResource, document.toByteArray());
                        }
                    }

                    // Send unavailable presence to the user who has sended the presence subscribe from the user
                    // who must receive the presence

                    presenceElement.setAttribute("from", to);
                    presenceElement.setAttribute("to", from);
                    presenceElement.setAttribute("type", "unavailable");

                    QStringList fromResourceList = keyList.filter(from, Qt::CaseInsensitive);
                    foreach (QString fromResource, fromResourceList)
                    {
                        m_userMap->value(fromResource)->getConnection()->write(document.toByteArray());
                        m_userMap->value(fromResource)->getConnection()->flush();

                        // Request acknowledgment of receipt
                        if (!m_userMap->value(fromResource)->getSmId().isEmpty())
                        {
                            sendReceiptRequest(fromResource, document.toByteArray());
                        }
                    }
                }
            }
            else
            {
                qDebug() << "J'entre ici";
                // Create contact on the roster of this contact
                m_rosterManager->addContactToRoster(to, Contact("", false, "", from, "", "none", QSet<QString>()));

                qDebug() << "presence broadcast data : " << document.toByteArray();
                foreach (QString toResource, toResourceList)
                {
                    m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
                    m_userMap->value(toResource)->getConnection()->flush();

                    // Request acknowledgment of receipt
                    if (!m_userMap->value(toResource)->getSmId().isEmpty())
                    {
                        sendReceiptRequest(toResource, document.toByteArray());
                    }
                }

                // Send unavailable presence to the user who has sended the presence subscribe from the user
                // who must receive the presence

                presenceElement.setAttribute("from", to);
                presenceElement.setAttribute("to", from);
                presenceElement.setAttribute("type", "unavailable");

                QStringList fromResourceList = keyList.filter(from, Qt::CaseInsensitive);
                foreach (QString fromResource, fromResourceList)
                {
                    m_userMap->value(fromResource)->getConnection()->write(document.toByteArray());
                    m_userMap->value(fromResource)->getConnection()->flush();

                    // Request acknowledgment of receipt
                    if (!m_userMap->value(fromResource)->getSmId().isEmpty())
                    {
                        sendReceiptRequest(fromResource, document.toByteArray());
                    }
                }
            }
        }
        else if (presenceType == "unsubscribe")
        {
            if (m_rosterManager->contactExists(to, from))
            {
                Contact userContact = m_rosterManager->getContact(to, from);
                if ((userContact.getSubscription() == "from") || (userContact.getSubscription() == "both"))
                {
                    qDebug() << "presence broadcast data : " << document.toByteArray();
                    foreach (QString toResource, toResourceList)
                    {
                        m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
                        m_userMap->value(toResource)->getConnection()->flush();

                        // Request acknowledgment of receipt
                        if (!m_userMap->value(toResource)->getSmId().isEmpty())
                        {
                            sendReceiptRequest(toResource, document.toByteArray());
                        }
                    }

                    Contact userContact = m_rosterManager->getContact(to, from);
                    if (userContact.getSubscription() == "from")
                        m_rosterManager->updateSubscriptionToContact(to, from, "none");

                    if (userContact.getSubscription() == "both")
                        m_rosterManager->updateSubscriptionToContact(to, from, "to");

                    m_rosterManager->updateAskAttributeToContact(to, from, "");

                    // Refresh user contact
                    userContact = m_rosterManager->getContact(to, from);

                    // Roster push
                    rosterPush(to, Utils::generateRosterPush(to, Utils::generateId(), from,
                                                             userContact.getName(),
                                                             userContact.getSubscription(),
                                                             userContact.getAsk(),
                                                             userContact.getApproved(),
                                                             userContact.getGroups()));

                    // send presence of type unavailable to the user who send the presence unsubcribe
                    presenceUnavailableBroadCast(from, to);
                }
            }
        }
        else if (presenceType == "subscribed")
        {
            if (m_rosterManager->contactExists(to, from))
            {
                Contact userContact = m_rosterManager->getContact(to, from);
                if (((userContact.getSubscription() == "none") || (userContact.getSubscription() == "from"))
                     && (userContact.getAsk() == "subscribe"))
                {
                    qDebug() << "presence broadcast data : " << document.toByteArray();
                    foreach (QString toResource, toResourceList)
                    {
                        m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
                        m_userMap->value(toResource)->getConnection()->flush();

                        // Request acknowledgment of receipt
                        if (!m_userMap->value(toResource)->getSmId().isEmpty())
                        {
                            sendReceiptRequest(toResource, document.toByteArray());
                        }
                    }

                    m_rosterManager->updateAskAttributeToContact(to, from, "");
                    m_rosterManager->updateSubscriptionToContact(to, from, "to");

                    // Refresh the contact
                    userContact = m_rosterManager->getContact(to, from);

                    // Send roster push
                    rosterPush(to, Utils::generateRosterPush(to, Utils::generateId(),
                                                             userContact.getJid(),
                                                             userContact.getName(),
                                                             userContact.getSubscription(),
                                                             userContact.getAsk(),
                                                             userContact.getApproved(),
                                                             userContact.getGroups()));

                    // Send the available presence stanza received from the user the presence
                    // subscribed to the user who received the presence subscribed.
                    presenceBroadCastFromContact(to, from);
                }
            }
        }
        else if (presenceType == "unsubscribed")
        {
            if (m_rosterManager->contactExists(to, from))
            {
                Contact userContact = m_rosterManager->getContact(to, from);
                if ((userContact.getSubscription() == "to") || (userContact.getSubscription() == "both"))
                {
                    qDebug() << "presence broadcast data : " << document.toByteArray();
                    foreach (QString toResource, toResourceList)
                    {
                        m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
                        m_userMap->value(toResource)->getConnection()->flush();

                        // Request acknowledgment of receipt
                        if (!m_userMap->value(toResource)->getSmId().isEmpty())
                        {
                            sendReceiptRequest(toResource, document.toByteArray());
                        }
                    }

                    Contact userContact = m_rosterManager->getContact(to, from);
                    if (userContact.getSubscription() == "to")
                        m_rosterManager->updateSubscriptionToContact(to, from, "none");

                    if (userContact.getSubscription() == "both")
                        m_rosterManager->updateSubscriptionToContact(to, from, "from");

                    m_rosterManager->updateAskAttributeToContact(to, from, "");

                    // Refresh user contact
                    userContact = m_rosterManager->getContact(to, from);

                    // Roster push
                    rosterPush(to, Utils::generateRosterPush(to, Utils::generateId(), from,
                                                             userContact.getName(),
                                                             userContact.getSubscription(),
                                                             userContact.getAsk(),
                                                             userContact.getApproved(),
                                                             userContact.getGroups()));
                }
            }
        }
    }
    else
    {
        QString from = presenceElement.attribute("from");
        QString to = presenceElement.attribute("to");

        // Save this presence subscription
        if (!presenceType.isEmpty() && (presenceType != "unavailable"))
            saveOfflinePresenceSubscription(Utils::getBareJid(from), Utils::getBareJid(to), data, presenceType);
    }
}

void StreamManager::presenceBroadCastFromContact(QString to, QString contactJid)
{    
    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());
    QStringList toResourceList = keyList.filter(to, Qt::CaseInsensitive);
    QStringList contactResourceList = keyList.filter(contactJid, Qt::CaseInsensitive);

    foreach (QString fromResource, contactResourceList)
    {
        QDomDocument document;
        document.setContent(m_userMap->value(fromResource)->getCurrentPresence());

        QDomElement presenceElement = document.documentElement();
        presenceElement.setAttribute("from", fromResource);
        foreach (QString toResource, toResourceList)
        {
            presenceElement.setAttribute("to", to);
            m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
            m_userMap->value(toResource)->getConnection()->flush();

            // Request acknowledgment of receipt
            if (!m_userMap->value(toResource)->getSmId().isEmpty())
            {
                sendReceiptRequest(toResource, document.toByteArray());
            }
        }
    }
}

void StreamManager::sendMessage(QString to, QByteArray message)
{
    bool offlineUser = true;
    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());

    if (to.contains("/"))
    {
        if (m_userMap->value(to)->getPresencePriority() >= 0)
        {
            m_userMap->value(to)->getConnection()->write(message);
            m_userMap->value(to)->getConnection()->flush();
            offlineUser = false;
        }
        else
        {
            QStringList toResourceList = keyList.filter(Utils::getBareJid(to), Qt::CaseInsensitive);
            if (!toResourceList.isEmpty())
            {
                foreach (QString toResource, toResourceList)
                {
                    if ((m_userMap->value(toResource)->getPresencePriority() >= 0))
                    {
                        offlineUser = false;
                        m_userMap->value(toResource)->getConnection()->write(message);
                        m_userMap->value(toResource)->getConnection()->flush();
                    }

                    // Request acknowledgment of receipt
                    if (!m_userMap->value(toResource)->getSmId().isEmpty())
                    {
                        sendReceiptRequest(toResource, message);
                    }
                }
            }
        }
    }
    else
    {
        QStringList toResourceList = keyList.filter(to, Qt::CaseInsensitive);
        foreach (QString toResource, toResourceList)
        {
            if ((m_userMap->value(toResource)->getPresencePriority() >= 0))
            {
                offlineUser = false;
                m_userMap->value(toResource)->getConnection()->write(message);
                m_userMap->value(toResource)->getConnection()->flush();
            }

            // Request acknowledgment of receipt
            if (!m_userMap->value(toResource)->getSmId().isEmpty())
            {
                sendReceiptRequest(toResource, message);
            }
        }
    }

    if (offlineUser)
    {
        QDomDocument document;
        document.setContent(message);

        if (document.documentElement().elementsByTagName("body").count() != 0)
        {
            QDateTime dateTime(QDateTime::currentDateTime());
            dateTime.setTimeSpec(Qt::UTC);
            saveOfflineMessage(Utils::getBareJid(document.documentElement().attribute("from")), to, message,
                               dateTime.toString("yyyy-MM-ddThh:mm:ss.zzz") + "Z");
        }
    }
}

void StreamManager::presenceProbeReply(QString to, QString from,
                                       bool directedPresenceProbe /* presence probe in the case of directed
                                       presence */)
{
    bool offlineContact = true;
    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());

    QStringList toResourceList = keyList.filter(to, Qt::CaseInsensitive);

    qDebug() << "presence probe reply toResourceList : " << toResourceList;

    if (directedPresenceProbe)
    {
        QStringList fromResourceList = keyList.filter(Utils::getBareJid(from), Qt::CaseInsensitive);

        qDebug() << "presence probe reply fromResourceList : " << fromResourceList;
        QDomDocument document;
        document.setContent(Utils::generatePresence("", "", to, Utils::generateId(), "", "", ""));
        QDomElement presenceElement = document.documentElement();

        foreach(QString fromResource, fromResourceList)
        {
            presenceElement.setAttribute("from", fromResource);
            foreach (QString toResource, toResourceList)
            {
                presenceElement.setAttribute("to", to);

                qDebug() << "presence probe reply  : " << document.toByteArray();
                m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
                m_userMap->value(toResource)->getConnection()->flush();

                // Request acknowledgment of receipt
                if (!m_userMap->value(toResource)->getSmId().isEmpty())
                {
                    sendReceiptRequest(toResource, document.toByteArray());
                }
            }
        }
        offlineContact = false;
    }
    else
    {
        presenceBroadCastFromContact(to, from);
//        QStringList fromResourceList = keyList.filter(from, Qt::CaseInsensitive);
//        qDebug() << "presence probe reply fromResourceList : " << fromResourceList;
//        if (!fromResourceList.isEmpty())
//        {
//            foreach(QString fromResource, fromResourceList)
//            {
//                QDomDocument document;
//                document.setContent(m_userPresenceInfos.value(fromResource));
//                QDomElement presenceElement = document.documentElement();

//                presenceElement.setAttribute("from", fromResource);
//                foreach (QString toResource, toResourceList)
//                {
//                    presenceElement.setAttribute("to", to);

//                    qDebug() << "presence probe reply  : " << document.toByteArray();
//                    m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
//                    m_userMap->value(toResource)->getConnection()->flush();
//                }
//            }
        offlineContact = false;
    }

    // Send presence of type unavailable to the user who send first presence probe
    if (offlineContact)
    {
        QByteArray data = Utils::generatePresence("unavailable", from, to, Utils::generateId(), "", "",
                                                  QMultiHash<QString, QString>());
        foreach (QString toResource, toResourceList)
        {
            qDebug() << "offline contact's of user probe sender reply : " << data;
            m_userMap->value(toResource)->getConnection()->write(data);
            m_userMap->value(toResource)->getConnection()->flush();

            // Request acknowledgment of receipt
            if (!m_userMap->value(toResource)->getSmId().isEmpty())
            {
                sendReceiptRequest(toResource, data);
            }
        }
    }
}

void StreamManager::presenceProbeToContact(QString to, QString from, bool directedPresenceProbe)
{
    if (m_userMap->value(from)->getCurrentPresence().isEmpty())
    {
        qDebug() << "presence probe to : " << to;
        presenceProbeReply(Utils::getBareJid(from), to, directedPresenceProbe);
    }
}

void StreamManager::directedPresence(QString from, QString to, QByteArray data)
{
    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());
    QStringList toResourceList = keyList.filter(to, Qt::CaseInsensitive);

    if (toResourceList.isEmpty())
    {
        foreach (QString toResource, toResourceList)
        {
            m_userMap->value(toResource)->getConnection()->write(data);
            m_userMap->value(toResource)->getConnection()->flush();

            // Request acknowledgment of receipt
            if (!m_userMap->value(toResource)->getSmId().isEmpty())
            {
                sendReceiptRequest(toResource, data);
            }
        }
        // Presence probe rules
        presenceProbeReply(from, to, true);
    }
}

void StreamManager::defaultListNameSetReply(QString jid, QString to, QString defaultListName, QString id)
{
    // There is already a default list name set for this user
    QStringList filter = m_resourceDefaultList->filter(jid);
    if (!filter.isEmpty() && (filter.value(0) != to))
    {
        QDomDocument document;
        QDomElement defaultElement = document.createElement("default");
        defaultElement.setAttribute("name", defaultListName);

        QByteArray errorData = Error::generateError("iq", "cancel", "conflict", "", "", id, defaultElement);

        m_userMap->value(to)->getConnection()->write(errorData);
        m_userMap->value(to)->getConnection()->flush();

        // Request acknowledgment of receipt
        if (!m_userMap->value(to)->getSmId().isEmpty())
        {
            sendReceiptRequest(to, errorData);
        }
    }
    else
    {
        m_resourceDefaultList->append(to);
        QDomDocument document;
        QDomElement iq = document.createElement("iq");
        iq.setAttribute("type", "result");
        iq.setAttribute("to", to);
        iq.setAttribute("id", id);
        document.appendChild(iq);

        m_userMap->value(to)->getConnection()->write(document.toByteArray());
        m_userMap->value(to)->getConnection()->flush();

        // Request acknowledgment of receipt
        if (!m_userMap->value(to)->getSmId().isEmpty())
        {
            sendReceiptRequest(to, document.toByteArray());
        }
    }
}

void StreamManager::presenceUnavailableBroadCast(QString to, QString from)
{
    QDomDocument document;
    document.setContent(Utils::generatePresence("unavailable", from, to, Utils::generateId(), "", "", ""));
    QDomElement presenceElement = document.documentElement();

    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());
    QStringList toResourceList = keyList.filter(to, Qt::CaseInsensitive);
    QStringList fromResourceList = keyList.filter(from, Qt::CaseInsensitive);

    foreach (QString fromResource, fromResourceList)
    {
        presenceElement.setAttribute("from", fromResource);
        foreach (QString toResource,toResourceList)
        {
            m_userMap->value(toResource)->getConnection()->write(document.toByteArray());
            m_userMap->value(toResource)->getConnection()->flush();

            // Request acknowledgment of receipt
            if (!m_userMap->value(toResource)->getSmId().isEmpty())
            {
                sendReceiptRequest(toResource, document.toByteArray());
            }
        }
    }
}

void StreamManager::lastActivityQuery(QString from, QString to, QString id, QString lastStatus)
{
    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());
    QStringList userResourceList = keyList.filter(Utils::getBareJid(to));

    if (userResourceList.isEmpty())
    {
        QByteArray errorData = Error::generateError("iq", "auth", "forbidden", Utils::getBareJid(to),
                                                             from, id, QDomElement());
        m_userMap->value(from)->getConnection()->write(errorData);
        m_userMap->value(from)->getConnection()->flush();

        // Request acknowledgment of receipt
        if (!m_userMap->value(from)->getSmId().isEmpty())
        {
            sendReceiptRequest(from, errorData);
        }
    }
    else
    {
        qDebug() << "last activity query : from = " << from << " to : " << to;
        QDomDocument document;
        QDomElement iq = document.createElement("iq");
        iq.setAttribute("from", to);
        iq.setAttribute("to", from);
        iq.setAttribute("type", "result");
        iq.setAttribute("id", id);

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "jabber:iq:last");
        query.setAttribute("seconds", (int)(m_userMap->value(to)->getConnectedTime()/1000));

        if (!lastStatus.isEmpty())
        {
            query.appendChild(document.createTextNode(lastStatus));
        }
        iq.appendChild(query);
        document.appendChild(iq);

        m_userMap->value(from)->getConnection()->write(document.toByteArray());
        m_userMap->value(from)->getConnection()->flush();

        // Request acknowledgment of receipt
        if (!m_userMap->value(from)->getSmId().isEmpty())
        {
            sendReceiptRequest(from, document.toByteArray());
        }
    }
}

void StreamManager::serverLastActivityQuery(QString from, QString to, QString id)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("from", to);
    iq.setAttribute("to", from);
    iq.setAttribute("type", "result");
    iq.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:last");
    query.setAttribute("seconds", (int)(m_serverTime.elapsed()/1000));

    iq.appendChild(query);
    document.appendChild(iq);

    m_userMap->value(from)->getConnection()->write(document.toByteArray());
    m_userMap->value(from)->getConnection()->flush();

    // Request acknowledgment of receipt
    if (!m_userMap->value(from)->getSmId().isEmpty())
    {
        sendReceiptRequest(from, document.toByteArray());
    }
}

void StreamManager::accountAvailableResourceQuery(QString from, QString accountJid, QString id)
{
    QStringList keyList = QStringList::fromSet(m_userMap->keys().toSet());
    QStringList accountResourceList = keyList.filter(Utils::getBareJid(accountJid));

    if (!accountResourceList.isEmpty())
    {
        QDomDocument document;

        QDomElement iq = document.createElement("iq");
        iq.setAttribute("from", accountJid);
        iq.setAttribute("to", from);
        iq.setAttribute("id", id);
        iq.setAttribute("type", "result");

        QDomElement query = document.createElement("query");
        query.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");

        foreach (QString accountResource, accountResourceList)
        {
            QDomElement item = document.createElement("item");
            item.setAttribute("jid", accountResource);
            query.appendChild(item);
        }
        iq.appendChild(query);
        document.appendChild(iq);

        // Route the document to the server host to the from value
    }
}

void StreamManager::inboundStanzaReceived(QString from)
{
    m_userMap->value(from)->setInboundStanzaReceivedCount(
                m_userMap->value(from)->getInboundStanzaReceivedCount() + 1);
}

void StreamManager::queryInboundStanzaReceived(QString from)
{
    QDomDocument document;
    QDomElement a = document.createElement("a");
    a.setAttribute("xmlns", "urn:xmpp:sm:3");
    a.setAttribute("h", m_userMap->value(from)->getInboundStanzaReceivedCount());
    document.appendChild(a);

    m_userMap->value(from)->getConnection()->write(document.toByteArray());
    m_userMap->value(from)->getConnection()->flush();
}

void StreamManager::sendReceiptRequest(QString to, QByteArray data)
{
    // save stream data
    saveStreamData(m_userMap->value(to)->getSmId(), data);

    QDomDocument document;
    QDomElement r = document.createElement("r");
    r.setAttribute("xmlns", "urn:xmpp:sm:3");
    document.appendChild(r);

    m_userMap->value(to)->getConnection()->write(document.toByteArray());
    m_userMap->value(to)->getConnection()->flush();
}

void StreamManager::slotSendReceiptRequest(QString to, QByteArray data)
{
    if (!m_userMap->value(to)->getSmId().isEmpty())
        sendReceiptRequest(to, data);
}

void StreamManager::acknowledgeReceiptServerStanza(QString from, int h)
{
    // delete this stanza in the stream temp file
    deleteStreamData(m_userMap->value(from)->getSmId(), h);
}

bool StreamManager::saveStreamData(QString smId, QByteArray data)
{
    return m_storageManager->getStorage()->saveStreamData(smId, data);
}

bool StreamManager::deleteStreamData(QString smId, int h)
{
    return m_storageManager->getStorage()->deleteStreamData(smId, h);
}

bool StreamManager::saveStreamPresencePriority(QString smId, int presencePriority)
{
    return m_storageManager->getStorage()->saveStreamPresencePriority(smId, presencePriority);
}

bool StreamManager::saveStreamPresenceStanza(QString smId, QByteArray presenceData)
{
    return m_storageManager->getStorage()->saveStreamPresenceStanza(smId, presenceData);
}

QList<QByteArray> StreamManager::getClientUnhandleStanza(QString smId)
{
    return m_storageManager->getStorage()->getClientUnhandleStanza(smId);
}

void StreamManager::saveOfflineMessage(QString from, QString to, QByteArray message, QString stamp)
{
    m_storageManager->getStorage()->saveOfflineMessage(from, to, message, stamp);
}

void StreamManager::saveOfflinePresenceSubscription(QString from, QString to, QByteArray presence,
                                                               QString presenceType)
{
    m_storageManager->getStorage()->saveOfflinePresenceSubscription(from, to, presence, presenceType);
}

QList<QVariant> StreamManager::getOfflinePresenceSubscription(QString jid)
{
    return m_storageManager->getStorage()->getOfflinePresenceSubscription(jid);
}

QMultiHash<QString, QByteArray> StreamManager::getAllOfflineMessage(QString jid)
{
    return m_storageManager->getStorage()->getAllOfflineMessage(jid);
}

void StreamManager::resumeStream(Connection *connection, QString prevId, int h)
{
    QString prevIdFilename = "temp/" + prevId + ".qjt";
    if (!QFile::exists(prevIdFilename))
    {
        connection->write(Error::generateSmError("item-not-found"));
        connection->flush();
    }
    else
    {
        // resend unhandle stanza
        // Create the user class with all its previous stream information.
    }
}


void StreamManager::run()
{
    // Entering in event loop
    exec();
}