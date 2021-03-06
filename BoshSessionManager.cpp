#include "BoshSessionManager.h"

BoshSessionManager::BoshSessionManager(QObject *parent, int boshPort, int xmppServerPort, bool crossDomainBosh) : QTcpServer(parent)
{
    m_sessionMap = new QMap<QString, BoshSession* >();
    m_boshPort = boshPort;
    m_xmppServerPort = xmppServerPort;
    m_crossDomainBosh = crossDomainBosh;
}

void BoshSessionManager::incomingConnection(qintptr socketDescriptor)
{
    Connection *connection = new Connection(0);
    connection->setSocketDescriptor(socketDescriptor);

    connect(connection, SIGNAL(readyRead()), this, SLOT(dataReceived()));
}

void BoshSessionManager::dataReceived()
{
    Connection *boshConnection = qobject_cast<Connection *>(sender());
    QMap<QByteArray, QByteArray> requestValues = Utils::parseHttpRequest(boshConnection->readAll());

    QDomDocument document;
    if (document.setContent(requestValues.value("body")))
    {
        if (document.firstChildElement().attribute("sid").isEmpty())
        {
            QDomElement bodyElement = document.firstChildElement();

            Connection *xmppServerConnection = new Connection(0);
            BoshSession *session = new BoshSession(this, xmppServerConnection,
                                                   bodyElement.attribute("content"),
                                                   bodyElement.attribute("from"),
                                                   bodyElement.attribute("hold").toInt(),
                                                   bodyElement.attribute("rid").toInt(),
                                                   bodyElement.attribute("to"),
                                                   bodyElement.attribute("route"),
                                                   bodyElement.attribute("wait").toInt(),
                                                   bodyElement.attribute("rid").toInt(),
                                                   bodyElement.attribute("xml:lang"),
                                                   Utils::generateId());

            // Enable cross domain for this Bosh session
            if (m_crossDomainBosh)
                session->setCrossDomainBosh(true);

            session->setBoshFirstConnection(boshConnection);

            connect(session, SIGNAL(sigCloseBoshSession(QString)), this, SLOT(closeBoshSession(QString)));
            m_sessionMap->insert(session->sid(), session);
            //session->start();

            xmppServerConnection->connectToHost(QHostInfo::fromName(session->host()).addresses().value(0), m_xmppServerPort);
        }
        else
        {
            QDomDocument document;
            if (document.setContent(requestValues.value("body")))
            {
                QString sid = document.firstChildElement().attribute("sid");
                if (document.documentElement().attribute("type") == "terminate")
                {
                    m_sessionMap->value(sid)->close();
                }
                else
                {
                    m_sessionMap->value(sid)->sessionRequest(document, boshConnection);
                }
            }
        }
    }
    else
    {
        // Cross domain request
        // If user enable cross domain in server config
        if (m_crossDomainBosh)
        {
            QDomDocument document;
            document.appendChild(document.createElement("crossDomain"));

            boshConnection->write(Utils::generateHttpResponseHeader(document.toByteArray(-1).count(), true) + document.toByteArray(-1));
            boshConnection->flush();
        }
        else
        {
            boshConnection->close();
        }
    }
}

void BoshSessionManager::closeBoshSession(QString sid)
{
    m_sessionMap->remove(sid);
}

void BoshSessionManager::startManage()
{
    if (listen(QHostAddress::Any, m_boshPort))
        qDebug() << "QJabberd Bosh connection manager listen on port : " << m_boshPort;
    else
        qDebug() << "QJabberd Bosh connection manager can not listen on port : " << m_boshPort;
}

void BoshSessionManager::stopManage()
{
    close();
}

void BoshSessionManager::restart()
{
    stopManage();
    startManage();
}

