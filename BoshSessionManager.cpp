#include "BoshSessionManager.h"

BoshSessionManager::BoshSessionManager(QObject *parent, int boshPort, int xmppServerPort) : QTcpServer(parent)
{
    m_sessionMap = new QMap<QString, BoshSession* >();
    m_boshPort = boshPort;
    m_xmppServerPort = xmppServerPort;
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
    document.setContent(requestValues.value("body"));
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
            QString sid = document.firstChildElement().attribute("sid");/*
            if ((m_sessionMap->value(sid)->activeConnection() == -1) ||
                    (m_sessionMap->value(sid)->activeConnection() == 2))
            {
                m_sessionMap->value(sid)->setActiveConnection(1);
                m_sessionMap->value(sid)->setBoshFirstConnection(boshConnection);
            }
            else
            {
                m_sessionMap->value(sid)->setActiveConnection(2);
                m_sessionMap->value(sid)->setBoshSecondConnection(boshConnection);
            }*/

            /*if (document.documentElement().attribute("xmpp:restart") == "true")
            {
                // Restart Stream on the XMPP Server
                m_sessionMap->value(sid)->initXmppServerStream();
                m_sessionMap->value(sid)->setRid(m_sessionMap->value(sid)->rid() + 1);
            }
            else */if (document.documentElement().attribute("type") == "terminate")
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

