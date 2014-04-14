
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

Stream::Stream()
{
    m_connection = new Connection(0);
}

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
    qDebug() << "stream encrypted";
    m_streamNegotiationManager->firstNegotiationSuccess(m_streamId);
}

void Stream::sslError(QList<QSslError> errors)
{
    qDebug() << "SSL Error : " << errors;

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
    QByteArray data = m_connection->readAll();

    m_xmlPaquet += data;

    if (m_xmlPaquet == "</stream:stream>")
    {
        qDebug() << "Data : " << m_xmlPaquet << endl;
        closeStream();
        return;
    }

    QList<QByteArray> documentContentlist = Utils::parseRequest(m_xmlPaquet);

    qDebug() << "document content list : " << documentContentlist;

    foreach (QByteArray documentContent, documentContentlist)
    {
        requestTreatment(documentContent);
    }
}

void Stream::requestTreatment(QByteArray xmlRequest)
{
    QDomDocument document;
    document.setContent(xmlRequest);

    if (document.documentElement().tagName() == "stream:stream")
    {
        streamReply(m_streamNegotiationManager->reply(xmlRequest, m_streamId));
    }
    else if (document.documentElement().tagName() == "starttls")
    {
        QByteArray answer = m_streamNegotiationManager->reply(xmlRequest, m_streamId);

        QList<QByteArray> answerList = answer.split(',');
        // Send <proceed/> reply to client
        streamReply(answerList.value(0));

        // We begin handshake
        m_connection->setLocalCertificate(answerList.value(1));
        m_connection->setPrivateKey(answerList.value(2));
        m_connection->startServerEncryption();
    }
    else if (document.documentElement().tagName() == "auth")
    {
        QByteArray answer = m_streamNegotiationManager->reply(xmlRequest, m_streamId);

        // Send first <challenge/> or error to client.
        streamReply(answer);
    }
    else if (document.documentElement().tagName() == "response")
    {
        QByteArray answer = m_streamNegotiationManager->reply(xmlRequest, m_streamId);

        // We send other <challenge/> or <success/> xml.
        streamReply(answer);
    }
    else if (document.documentElement().tagName() == "abort")
    {
        QByteArray answer = m_streamNegotiationManager->reply(xmlRequest, m_streamId);
        streamReply(answer);
    }
    else if (document.documentElement().tagName() == "enable")
    {
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
        QByteArray answer;
        if (document.documentElement().firstChild().toElement().tagName() == "bind")
        {
            answer = m_streamNegotiationManager->reply(xmlRequest, m_streamId);
        }
        else
        {
            emit sigInboundStanzaReceived(m_fullJid);
            answer = m_iqManager->parseIQ(xmlRequest, m_fullJid, m_host);
        }

        // We send iq reply
        streamReply(answer);
    }
    else if (document.documentElement().tagName() == "presence")
    {
        emit sigInboundStanzaReceived(m_fullJid);
        QByteArray answer = m_presenceManager->parsePresence(xmlRequest, m_fullJid);

        // We send presence reply
        streamReply(answer);
        ++m_inboundStanzaCount;
    }
    else if (document.documentElement().tagName() == "message")
    {
        emit sigInboundStanzaReceived(m_fullJid);
        QByteArray answer = m_messageManager->parseMessage(xmlRequest, m_fullJid);

        // We send presence reply
        streamReply(answer);
        ++m_inboundStanzaCount;
    }
    else if (document.documentElement().tagName() == "r")
    {
        emit sigQueryInboundStanzaReceived(m_fullJid);

        streamReply(QByteArray());
    }
    else if (document.documentElement().tagName() == "a")
    {
        int h = document.documentElement().attribute("h").toInt();
        emit sigAcknowledgeReceiptServerStanza(m_fullJid, h);
    }
    else if (document.documentElement().tagName() == "resume")
    {
        emit sigResumeStream(m_connection, document.documentElement().attribute("previd"),
                             document.documentElement().attribute("h").toInt());
    }
    m_xmlPaquet.clear();
}

/*
 * This function reply to client
 */
void Stream::streamReply(QByteArray answer)
{
    qDebug() << "XML reply : " << answer << endl;

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

    //delete m_streamNegotiationManager;
}

void Stream::setHost(QString host)
{
    m_host = host;
}
