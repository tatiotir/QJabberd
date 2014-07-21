
/*
 *
 *
 *
 */


#include "Stream.h"

Stream::Stream(QString streamId, Connection *connection,
               IqManager *iqManager, PresenceManager *presenceManager,
               MessageManager *messageManager, RosterManager *rosterManager,
               StreamNegotiationManager *streamNegotiationManager, BlockingCommandManager *blockingCmdManager)
{
    m_streamId = streamId;
    m_connection = connection;
    m_iqManager = iqManager;
    m_presenceManager = presenceManager;
    m_messageManager = messageManager;
    m_rosterManager = rosterManager;
    m_streamNegotiationManager = streamNegotiationManager;
    m_blockingCmdManager = blockingCmdManager;

    m_connection->ignoreSslErrors();

    connect(m_connection, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(m_connection, SIGNAL(encrypted()), this, SLOT(streamEncrypted()));
    //connect(m_connection, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslError(QList<QSslError>)));
    //connect(m_connection, SIGNAL(disconnected()), this, SLOT(closeStream()));
}

//Stream::Stream()
//{
//    m_connection = new Connection(0);
//}

Stream::~Stream()
{
    delete m_connection;
}

void Stream::closeStream()
{
    if (!m_fullJid.isEmpty())
    {
        emit sigOfflineUser(m_fullJid);
        sendUnavailablePresence();
    }
    streamReply(QByteArray("</stream:stream>"));

    qDebug() << "Info : Client " << Utils::getBareJid(m_fullJid) << " disconnected.";
    emit sigCloseStream(m_fullJid);
}

void Stream::streamEncrypted()
{
    m_streamNegotiationManager->firstNegotiationSuccess(m_streamId);
}

void Stream::sslError(QList<QSslError> errors)
{
    QDomDocument document;

    QDomElement failure = document.createElement("failure");
    failure.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");

    document.appendChild(failure);

    // send <failure/> reply
    streamReply(document.toByteArray());

    // close the stream
    closeStream();
}

void Stream::dataReceived()
{
    m_xmlPaquet.append(m_connection->readAll());

    if (m_xmlPaquet == "</stream:stream>")
    {
        closeStream();
    }
    else if (m_xmlPaquet.contains("<stream:stream"))
    {
        if (m_xmlPaquet.contains("<?xml"))
        {
            int i = 0;
            while ((m_xmlPaquet.at(i) != '>') && (m_xmlPaquet.count() != i))
                ++i;
            m_xmlPaquet.remove(0, i + 1);
        }

        QByteArray content = m_xmlPaquet + "</stream:stream>";

        QDomDocument document;
        if (document.setContent(content))
        {
            requestTreatment(document);
            m_xmlPaquet.clear();
        }
    }
    else
    {
        QList<QByteArray> requestList = Utils::parseRequest(m_xmlPaquet);
        if (!requestList.isEmpty())
        {
            foreach (QByteArray request, requestList)
            {
                m_xmlPaquet.remove(m_xmlPaquet.indexOf(request), request.length() + 1);
                QDomDocument document;
                document.setContent(request);
                requestTreatment(document);
            }
        }
    }
}

void Stream::requestTreatment(QDomDocument document)
{
    if (!m_streamNegotiationManager->bindFeatureProceed(m_streamId) &&
            (document.documentElement().firstChildElement().attribute("xmlns") != "jabber:iq:register") &&
            (document.documentElement().firstChildElement().tagName() != "bind") &&
            (document.documentElement().firstChildElement().tagName() != "session"))
    {
        // qDebug() << "Client : " << document.toByteArray();
        if ((document.documentElement().tagName() == "starttls")
                && !m_streamNegotiationManager->firstFeatureProceed(m_streamId))
        {
            QByteArray answer = m_streamNegotiationManager->reply(document, m_streamId);

            QList<QByteArray> answerList = answer.split('#');
            // Send <proceed/> reply to client
            streamReply(answerList.value(0));

            // We begin handshake
            m_connection->setLocalCertificate(answerList.value(1));
            m_connection->setPrivateKey(answerList.value(2));
            m_connection->startServerEncryption();
        }
        else
        {
            streamReply(m_streamNegotiationManager->reply(document, m_streamId));
        }
    }
    else if (document.documentElement().tagName() == "enable")
    {
        // qDebug() << "Client : " << document.toByteArray();
        if (m_streamNegotiationManager->bindFeatureProceed(m_streamId))
        {
            QDomDocument document;
            QDomElement enabled = document.createElement("enabled");
            enabled.setAttribute("xmlns", "urn:xmpp:sm:3");
            enabled.setAttribute("resume", "true");

            QString id = Utils::generateId();
            enabled.setAttribute("id", id);
            document.appendChild(enabled);

            // enable stream management
            streamReply(document.toByteArray());

            emit sigEnableStreamManagement(m_fullJid, id);
        }
        else
        {
            streamReply(Error::generateSmError("unexpected-request"));
        }
    }
    // Simple user authentification method of xmpp protocol which use jabber:iq:auth as namespace
    else if (document.documentElement().tagName() == "iq")
    {
        //qDebug() << "Client : " << document.toByteArray();
        emit sigInboundStanzaReceived(m_fullJid);
        QByteArray answer = m_iqManager->parseIQ(document, m_fullJid, m_host, m_streamId);

        // We send iq reply
        streamReply(answer);
        ++m_inboundStanzaCount;
    }
    else if (document.documentElement().tagName() == "presence")
    {
        // qDebug() << "Client : " << document.toByteArray();
        emit sigInboundStanzaReceived(m_fullJid);
        QByteArray answer = m_presenceManager->parsePresence(document, m_fullJid);

        // We send presence reply
        streamReply(answer);
        ++m_inboundStanzaCount;
    }
    else if (document.documentElement().tagName() == "message")
    {
        // qDebug() << "Client : " << document.toByteArray();
        emit sigInboundStanzaReceived(m_fullJid);
        QByteArray answer = m_messageManager->parseMessage(document, m_fullJid);

        // We send presence reply
        streamReply(answer);
        ++m_inboundStanzaCount;
    }
    else if (document.documentElement().tagName() == "r")
    {
        // qDebug() << "Client : " << document.toByteArray();
        emit sigQueryInboundStanzaReceived(m_fullJid);
        streamReply(QByteArray());
    }
    else if (document.documentElement().tagName() == "a")
    {
        // qDebug() << "Client : " << document.toByteArray();
        int h = document.documentElement().attribute("h").toInt();
        emit sigAcknowledgeReceiptServerStanza(m_fullJid, h);
    }
    else if (document.documentElement().tagName() == "resume")
    {
        // qDebug() << "Client : " << document.toByteArray();
        emit sigResumeStream(m_connection, document.documentElement().attribute("previd"),
                             document.documentElement().attribute("h").toInt());
    }
}

/*
 * This function reply to client
 */
void Stream::streamReply(QByteArray answer)
{
    m_connection->write(answer);
    m_connection->flush();
}

void Stream::sendUnavailablePresence()
{
    QList<Contact> contactList = m_rosterManager->getContactsList(Utils::getBareJid(m_fullJid));

    QDomDocument document = PresenceManager::generatePresence("unavailable", m_fullJid, "", Utils::generateId(), "", "",
                                                QMultiHash<QString, QString>());
    for (int i = 0; i < contactList.count(); ++i)
    {
        Contact contact = contactList.value(i);
        if ((contact.getSubscription() == "from") || (contact.getSubscription() == "both"))
        {
            document.documentElement().setAttribute("to", contact.getJid());
            // Send presence to this contact
            emit sigPresenceBroadCast(contact.getJid(), document);
        }
    }
    document.documentElement().setAttribute("from", m_fullJid);
    document.documentElement().setAttribute("to", Utils::getBareJid(m_fullJid));
    emit sigPresenceBroadCast(Utils::getBareJid(m_fullJid), document);
}

void Stream::bindFeatureNegotiated(QString fullJid)
{
    m_fullJid = fullJid;
    qDebug() << "Info : New Client connected." << " Authenticated as " << Utils::getBareJid(fullJid) << endl;
    emit sigBindFeatureNegotiated(fullJid, this);
}

Connection* Stream::getConnection()
{
    return m_connection;
}

void Stream::setHost(QString host)
{
    m_host = host;
}
