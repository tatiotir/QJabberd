#include "BoshSession.h"

const QString BoshSession::m_charsets = "utf-8";
const int BoshSession::m_inactivity = 60;
const int BoshSession::m_polling = 5;
const int BoshSession::m_request = 2;
const int BoshSession::m_maxpause = 120;
const QString BoshSession::m_version = "1.6";

BoshSession::BoshSession(QObject *parent, Connection *xmppServerConnection,
                         QString content, QString from, int hold, int rid, QString host, QString route,
                         int wait, int ack, QString xmlLang, QString sid) : QObject(parent)
{
    m_boshFirstConnection = new Connection(0);
    m_boshSecondConnection = new Connection(0);
    m_xmppServerConnection = xmppServerConnection;
    m_content = content;
    m_from = from;
    m_hold = hold;
    m_rid = rid;
    m_host = host;
    m_route = route;
    m_wait = wait;
    m_ack = ack;
    m_xmlLang = xmlLang;
    m_sid = sid;
    m_saslNegotiated = false;
    m_nbRequest = 0;
    m_xmlPaquet = QByteArray();
    m_fullJid = QString();

    connect(m_xmppServerConnection, SIGNAL(connected()), this, SLOT(initXmppServerStream()));
    connect(m_xmppServerConnection, SIGNAL(readyRead()), this, SLOT(xmppServerDataReceived()));
    connect(m_xmppServerConnection, SIGNAL(disconnected()), this, SLOT(deleteBoshSession()));

    m_keepAliveTimer = new QTimer();
    m_keepAliveTimer->setInterval(m_wait*1000);
    connect(m_keepAliveTimer, SIGNAL(timeout()), this, SLOT(sendKeepAlive()));
}

Connection *BoshSession::boshSecondConnection() const
{
    return m_boshSecondConnection;
}

void BoshSession::setBoshSecondConnection(Connection *boshSecondConnection)
{
    m_boshSecondConnection = boshSecondConnection;
}

Connection *BoshSession::boshFirstConnection() const
{
    return m_boshFirstConnection;
}

void BoshSession::setBoshFirstConnection(Connection *boshFirstConnection)
{
    m_boshFirstConnection = boshFirstConnection;
}

int BoshSession::nbRequest() const
{
    return m_nbRequest;
}

void BoshSession::setNbRequest(int nbRequest)
{
    m_nbRequest = nbRequest;
}

QString BoshSession::fullJid() const
{
    return m_fullJid;
}

void BoshSession::setFullJid(const QString &fullJid)
{
    m_fullJid = fullJid;
}

void BoshSession::xmppServerDataReceived()
{
    m_xmlPaquet.append(m_xmppServerConnection->readAll());
    if (m_xmlPaquet == "</stream:stream>")
    {
        close();
    }
    else if (m_xmlPaquet.contains("<stream:stream"))
    {
        int i = 0;
        while ((m_xmlPaquet.at(i) != '>') && (m_xmlPaquet.count() != i))
            ++i;
        m_xmlPaquet.remove(0, i + 1);

        if (!m_saslNegotiated)
        {
            int indexTls = m_xmlPaquet.indexOf("<starttls");
            int j = indexTls + 9;

            while ((m_xmlPaquet.at(j) != '>') && (m_xmlPaquet.count() != j))
                   ++j;
            m_xmlPaquet.remove(indexTls, j - indexTls + 1);

            qDebug() << "Stream : " << m_xmlPaquet;

            QDomDocument document;
            if (document.setContent(m_xmlPaquet))
            {
                boshSessionInitiationReply(document);
            }
        }
        else
        {
            QDomDocument document;
            if (document.setContent(m_xmlPaquet))
            {
                boshSessionRestartReply(document);
            }
        }
        m_xmlPaquet.clear();
    }
    else
    {
        if (!m_xmlPaquet.isEmpty())
        {
            m_xmppServerResponseList = Utils::parseRequest(m_xmlPaquet);

            //qDebug() << "Nb request : " << m_nbRequest << " response : " << m_xmppServerResponseList.count();
            if (m_xmppServerResponseList.count() == m_nbRequest)
            {
                boshSessionRequestReply();
                m_nbRequest = 0;
            }
            else if ((m_nbRequest == 0) || (m_xmppServerResponseList.count() != m_nbRequest))
            {
                if (!m_xmppServerResponseList.isEmpty())
                {
                    boshSessionReply();
                }
            }
            m_xmppServerResponseList.clear();
            m_xmlPaquet.clear();
        }
    }
}

void BoshSession::requestReply(QByteArray reply)
{
    if (m_boshFirstConnection->isOpen() && (m_boshFirstConnection->openMode() != QIODevice::NotOpen))
    {
        qDebug() << "Reply : " << reply;
        m_boshFirstConnection->write(reply);
        m_boshFirstConnection->flush();
    }

    // After wait second we send keep alive HTTP POST
    m_keepAliveTimer->start();
}

void BoshSession::emptyRequestReply(QByteArray reply)
{
    if (m_boshSecondConnection->isOpen() && (m_boshSecondConnection->openMode() != QIODevice::NotOpen))
    {
        m_boshSecondConnection->write(reply);
        m_boshSecondConnection->flush();
    }
    else
    {
        m_boshFirstConnection->write(reply);
        m_boshFirstConnection->flush();
    }

}

Connection *BoshSession::xmppServerConnection() const
{
    return m_xmppServerConnection;
}

void BoshSession::setXmppServerConnection(Connection *xmppServerConnection)
{
    m_xmppServerConnection = xmppServerConnection;
}

QString BoshSession::content() const
{
    return m_content;
}

void BoshSession::setContent(const QString &content)
{
    m_content = content;
}
QString BoshSession::from() const
{
    return m_from;
}

void BoshSession::setFrom(const QString &from)
{
    m_from = from;
}
int BoshSession::hold() const
{
    return m_hold;
}

void BoshSession::setHold(int hold)
{
    m_hold = hold;
}
int BoshSession::rid() const
{
    return m_rid;
}

void BoshSession::setRid(int rid)
{
    m_rid = rid;
}

QString BoshSession::host() const
{
    return m_host;
}

void BoshSession::setHost(const QString &host)
{
    m_host = host;
}
QString BoshSession::route() const
{
    return m_route;
}

void BoshSession::setRoute(const QString &route)
{
    m_route = route;
}

int BoshSession::wait() const
{
    return m_wait;
}

void BoshSession::setWait(int wait)
{
    m_wait = wait;
}
int BoshSession::ack() const
{
    return m_ack;
}

void BoshSession::setAck(int ack)
{
    m_ack = ack;
}
QString BoshSession::xmlLang() const
{
    return m_xmlLang;
}

void BoshSession::setXmlLang(const QString &xmlLang)
{
    m_xmlLang = xmlLang;
}

QString BoshSession::sid() const
{
    return m_sid;
}

void BoshSession::setSid(const QString &sid)
{
    m_sid = sid;
}

void BoshSession::sessionRequest(QDomDocument request, Connection *connection)
{
    if (request.documentElement().attribute("type") == "terminate")
    {
        m_boshFirstConnection = connection;
        close();
    }
    else if (request.documentElement().attribute("xmpp:restart") == "true")
    {
        initXmppServerStream();
        m_boshFirstConnection = connection;
    }
    else if (request.documentElement().childNodes().isEmpty())
    {
        m_boshSecondConnection = connection;
        QTimer::singleShot(30000, this, SLOT(sendKeepAlive()));
    }
    else
    {
        m_boshFirstConnection = connection;
        m_nbRequest = 0;
        QDomNodeList nodeList = request.documentElement().childNodes();
        QList<QDomDocument> requestList;
        for (int i = 0; i < nodeList.count(); ++i)
        {
            QDomDocument requestDocument;
            requestDocument.appendChild(requestDocument.importNode(nodeList.at(i), true));
            requestList << requestDocument;
        }

        foreach (QDomDocument request, requestList)
        {
            m_xmppServerConnection->write(request.toByteArray());
            m_xmppServerConnection->flush();
            if ((request.documentElement().tagName() == "message")
                    || (request.documentElement().attribute("type") == "result")
                    || (request.documentElement().tagName() == "presence"))
            {
                QDomDocument document;
                QDomElement bodyElement = document.createElement("body");
                bodyElement.setAttribute("sid", m_sid);
                bodyElement.setAttribute("xmlns:stream", "http://etherx.jabber.org/streams");
                bodyElement.setAttribute("xmlns", "http://jabber.org/protocol/httpbind");

                document.appendChild(bodyElement);
                requestReply(Utils::generateHttpResponseHeader(document.toByteArray(-1).count(), m_crossDomainBosh) + document.toByteArray(-1));
            }
            else
            {
                ++m_nbRequest;
            }
        }
    }
}

void BoshSession::boshSessionInitiationReply(QDomDocument document)
{
    QDomDocument doc;
    QDomElement bodyElement = doc.createElement("body");
    bodyElement.setAttribute("wait", m_wait);
    bodyElement.setAttribute("inactivity", BoshSession::m_inactivity);
    bodyElement.setAttribute("polling", BoshSession::m_polling);
    bodyElement.setAttribute("request", BoshSession::m_request);
    bodyElement.setAttribute("hold", m_hold);
    bodyElement.setAttribute("from", m_host);
    bodyElement.setAttribute("sid", m_sid);
    //bodyElement.setAttribute("ack", session->ack());
    //bodyElement.setAttribute("maxpause", BoshSession::m_maxpause);
    //bodyElement.setAttribute("charserts", BoshSession::m_charsets);
    bodyElement.setAttribute("xmlns:stream", "http://etherx.jabber.org/streams");
    bodyElement.setAttribute("xmpp:version", "1.0");
    bodyElement.setAttribute("xmlns:xmpp", "urn:xmpp:xbosh");
    bodyElement.setAttribute("ver", BoshSession::m_version);
    bodyElement.setAttribute("xmlns", "http://jabber.org/protocol/httpbind");

    bodyElement.appendChild(document.documentElement());
    doc.appendChild(bodyElement);

    requestReply(Utils::generateHttpResponseHeader(doc.toByteArray(-1).count(), m_crossDomainBosh) + doc.toByteArray(-1));
}

void BoshSession::boshSessionRestartReply(QDomDocument document)
{
    QDomDocument doc;
    QDomElement bodyElement = doc.createElement("body");
    //bodyElement.setAttribute("sid", m_sid);
    bodyElement.setAttribute("xmpp:version", "1.0");
    bodyElement.setAttribute("xmlns:stream", "http://etherx.jabber.org/streams");
    bodyElement.setAttribute("xmlns", "http://jabber.org/protocol/httpbind");
    bodyElement.setAttribute("xmlns:xmpp", "urn:xmpp:xbosh");

    bodyElement.appendChild(document.documentElement());
    doc.appendChild(bodyElement);

    requestReply(Utils::generateHttpResponseHeader(doc.toByteArray(-1).count(), m_crossDomainBosh) + doc.toByteArray(-1));
}

void BoshSession::boshSessionRequestReply()
{
    if (m_xmppServerResponseList.value(0).contains("<success"))
    {
        m_saslNegotiated = true;
    }

    if (m_xmppServerResponseList.value(0).contains("<bind"))
    {
        QDomDocument document;
        document.setContent(m_xmppServerResponseList.value(0));
        m_fullJid = document.documentElement().elementsByTagName("jid").item(0).toElement().text();
        qDebug() << "Info : " << "New BOSH session, assigned sid " << m_sid << ". Authenticated as " << Utils::getBareJid(m_fullJid);
    }

    QDomDocument replyDocument;
    QDomElement bodyElement = replyDocument.createElement("body");
    bodyElement.setAttribute("sid", m_sid);
    bodyElement.setAttribute("xmlns:stream", "http://etherx.jabber.org/streams");
    bodyElement.setAttribute("xmlns", "http://jabber.org/protocol/httpbind");

    foreach (QByteArray response, m_xmppServerResponseList)
    {
        QDomDocument responseDocument;
        responseDocument.setContent(response);

        bodyElement.appendChild(responseDocument.documentElement());
    }
    replyDocument.appendChild(bodyElement);
    requestReply(Utils::generateHttpResponseHeader(replyDocument.toByteArray(-1).count(), m_crossDomainBosh) + replyDocument.toByteArray(-1));
}

void BoshSession::boshSessionReply()
{
    QDomDocument replyDocument;
    QDomElement bodyElement = replyDocument.createElement("body");
    bodyElement.setAttribute("sid", m_sid);
    bodyElement.setAttribute("xmlns:stream", "http://etherx.jabber.org/streams");
    bodyElement.setAttribute("xmlns", "http://jabber.org/protocol/httpbind");

    foreach (QByteArray response, m_xmppServerResponseList)
    {
        QDomDocument responseDocument;
        responseDocument.setContent(response);

        bodyElement.appendChild(responseDocument.documentElement());
    }
    replyDocument.appendChild(bodyElement);
    emptyRequestReply(Utils::generateHttpResponseHeader(replyDocument.toByteArray(-1).count(), m_crossDomainBosh) + replyDocument.toByteArray(-1));
}

void BoshSession::sendKeepAlive()
{
    QDomDocument document;
    QDomElement bodyElement = document.createElement("body");
    bodyElement.setAttribute("sid", m_sid);
    bodyElement.setAttribute("xmlns:stream", "http://etherx.jabber.org/streams");
    bodyElement.setAttribute("xmlns", "http://jabber.org/protocol/httpbind");

    document.appendChild(bodyElement);
    emptyRequestReply(Utils::generateHttpResponseHeader(document.toByteArray(-1).count(), m_crossDomainBosh) + document.toByteArray(-1));
    //sendRequest();
}

void BoshSession::close()
{
    QDomDocument document;
    QDomElement bodyElement = document.createElement("body");
    bodyElement.setAttribute("sid", m_sid);
    bodyElement.setAttribute("xmlns:stream", "http://etherx.jabber.org/streams");
    bodyElement.setAttribute("xmlns", "http://jabber.org/protocol/httpbind");
    bodyElement.setAttribute("type", "terminate");

    document.appendChild(bodyElement);
    requestReply(Utils::generateHttpResponseHeader(document.toByteArray(-1).count(), m_crossDomainBosh) + document.toByteArray(-1));
    emptyRequestReply(Utils::generateHttpResponseHeader(document.toByteArray(-1).count(), m_crossDomainBosh) + document.toByteArray(-1));

    qDebug() << "Info : BOSH client " << Utils::getBareJid(m_fullJid) << " disconnected.";
    m_xmppServerConnection->write("</stream:stream>");
    m_xmppServerConnection->flush();
    m_xmppServerConnection->disconnectFromHost();

    m_boshFirstConnection->close();
    m_boshSecondConnection->close();
    m_keepAliveTimer->stop();
}

void BoshSession::deleteBoshSession()
{
    emit sigCloseBoshSession(m_sid);
}
bool BoshSession::crossDomainBosh() const
{
    return m_crossDomainBosh;
}

void BoshSession::setCrossDomainBosh(bool crossDomainBosh)
{
    m_crossDomainBosh = crossDomainBosh;
}


BoshSession::~BoshSession()
{
    delete m_keepAliveTimer;
    delete m_boshFirstConnection;
    delete m_boshSecondConnection;
    delete m_xmppServerConnection;
}

void BoshSession::initXmppServerStream()
{
    QByteArray xmlStream = "<?xml version='1.0'?>\n";
    xmlStream += "<stream:stream to='" + m_host + "' version='1.0' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>";
    m_xmppServerConnection->write(xmlStream);
    m_xmppServerConnection->flush();
}
