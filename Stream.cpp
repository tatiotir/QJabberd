
/*
 *
 *
 *
 */


#include "Stream.h"

Stream::Stream(QObject *parent, QString streamId, Connection *connection,
               IQManager *iqManager, PresenceManager *presenceManager,
               MessageManager *messageManager, RosterManager *rosterManager,
               StreamNegotiationManager *streamNegotiationManager) :
    QThread(parent)
{
    m_streamId = streamId;
    m_connection = connection;
    m_iqManager = iqManager;
    m_presenceManager = presenceManager;
    m_messageManager = messageManager;
    m_rosterManager = rosterManager;
    m_streamNegotiationManager = streamNegotiationManager;

    m_document = new QDomDocument();
    m_connection->ignoreSslErrors();

    connect(m_connection, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(m_connection, SIGNAL(encrypted()), this, SLOT(streamEncrypted()));
    connect(m_connection, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslError(QList<QSslError>)));
    //connect(m_connection, SIGNAL(disconnected()), this, SLOT(closeStream()));
}

//Stream::Stream()
//{
//    m_connection = new Connection(0);
//}

void Stream::run()
{
    // Entering in event loop
    exec();
}

void Stream::closeStream()
{
    if (!m_fullJid.isEmpty())
    {
        emit sigOfflineUser(m_fullJid);
        sendUnavailablePresence();
    }
    streamReply(QByteArray("</stream:stream>"));
    exit(0);
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
    //streamReply(document.toByteArray());

    // close the stream
    //closeStream();
}

void Stream::dataReceived()
{
    m_xmlPaquet.append(m_connection->readAll());
    qDebug() << "paquet : " << m_xmlPaquet;

    if (m_xmlPaquet == "</stream:stream>")
    {
        closeStream();
    }
    else
    {
        QList<QByteArray> documentContentlist = Utils::parseRequest(m_xmlPaquet.trimmed());
        if (!documentContentlist.isEmpty())
        {
            foreach (QByteArray documentContent, documentContentlist)
            {
                m_xmlPaquet.remove(0, documentContent.count());
                requestTreatment(documentContent);
            }
        }
    }
}

void Stream::requestTreatment(QByteArray xmlRequest)
{
    QDomDocument document;
    document.setContent(xmlRequest);

    if (!m_streamNegotiationManager->bindFeatureProceed(m_streamId) &&
            (document.documentElement().firstChildElement().attribute("xmlns") != "jabber:iq:register") &&
            (document.documentElement().firstChildElement().tagName() != "bind") &&
            (document.documentElement().firstChildElement().tagName() != "session"))
    {
        qDebug() << "Client : " << xmlRequest;
        if ((document.documentElement().tagName() == "starttls")
                && !m_streamNegotiationManager->firstFeatureProceed(m_streamId))
        {
            QByteArray answer = m_streamNegotiationManager->reply(xmlRequest, m_streamId);

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
            streamReply(m_streamNegotiationManager->reply(xmlRequest, m_streamId));
        }
        m_xmlPaquet.clear();
    }
    else if (document.documentElement().tagName() == "enable")
    {
        qDebug() << "Client : " << xmlRequest;
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
        m_xmlPaquet.clear();
    }
    // Simple user authentification method of xmpp protocol which use jabber:iq:auth as namespace
    else if (document.documentElement().tagName() == "iq")
    {
        qDebug() << "Client : " << xmlRequest;
        emit sigInboundStanzaReceived(m_fullJid);
        QByteArray answer = m_iqManager->parseIQ(xmlRequest, m_fullJid, m_host, m_streamId);

        // We send iq reply
        streamReply(answer);
        m_xmlPaquet.clear();
    }
    else if (document.documentElement().tagName() == "presence")
    {
        qDebug() << "Client : " << xmlRequest;
        emit sigInboundStanzaReceived(m_fullJid);
        QByteArray answer = m_presenceManager->parsePresence(xmlRequest, m_fullJid);

        // We send presence reply
        streamReply(answer);
        ++m_inboundStanzaCount;
        m_xmlPaquet.clear();
    }
    else if (document.documentElement().tagName() == "message")
    {
        qDebug() << "Client : " << xmlRequest;
        emit sigInboundStanzaReceived(m_fullJid);
        QByteArray answer = m_messageManager->parseMessage(xmlRequest, m_fullJid);

        // We send presence reply
        streamReply(answer);
        ++m_inboundStanzaCount;
        m_xmlPaquet.clear();
    }
    else if (document.documentElement().tagName() == "r")
    {
        qDebug() << "Client : " << xmlRequest;
        emit sigQueryInboundStanzaReceived(m_fullJid);
        streamReply(QByteArray());
        m_xmlPaquet.clear();
    }
    else if (document.documentElement().tagName() == "a")
    {
        qDebug() << "Client : " << xmlRequest;
        int h = document.documentElement().attribute("h").toInt();
        emit sigAcknowledgeReceiptServerStanza(m_fullJid, h);
        m_xmlPaquet.clear();
    }
    else if (document.documentElement().tagName() == "resume")
    {
        qDebug() << "Client : " << xmlRequest;
        emit sigResumeStream(m_connection, document.documentElement().attribute("previd"),
                             document.documentElement().attribute("h").toInt());
        m_xmlPaquet.clear();
    }
}

/*
 * This function reply to client
 */
void Stream::streamReply(QByteArray answer)
{
    qDebug() << endl << "Server : " << answer << endl;

    m_connection->write(answer);
    m_connection->flush();
}

void Stream::sendUnavailablePresence()
{
    QList<Contact> contactList = m_rosterManager->getContactsList(Utils::getBareJid(m_fullJid));

    QDomDocument document;
    document.setContent(Utils::generatePresence("unavailable", m_fullJid, "", Utils::generateId(), "", "",
                                                QMultiHash<QString, QString>()));
    for (int i = 0; i < contactList.count(); ++i)
    {
        Contact contact = contactList.value(i);
        if ((contact.getSubscription() == "from") || (contact.getSubscription() == "both"))
        {
            document.documentElement().setAttribute("to", contact.getJid());
            // Send presence to this contact
            emit sigPresenceBroadCast(contact.getJid(), document.toByteArray());
        }
    }
    document.documentElement().setAttribute("from", m_fullJid);
    document.documentElement().setAttribute("to", Utils::getBareJid(m_fullJid));
    emit sigPresenceBroadCast(Utils::getBareJid(m_fullJid), document.toByteArray());
}

void Stream::bindFeatureNegotiated(QString fullJid)
{
    m_fullJid = fullJid;
    emit sigBindFeatureNegotiated(fullJid, m_connection);
}

void Stream::setHost(QString host)
{
    m_host = host;
}
