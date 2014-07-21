#include "Server.h"

Server::Server()
{
    QFile configFile("qjabberd.config");
    if (!configFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error in the server configuration file";
    }
    else
    {
        QJsonDocument configDocument = QJsonDocument::fromJson(configFile.readAll());
        m_configuration = new QJsonObject(configDocument.object());
        configFile.close();
    }

    m_conManager = new ConnectionManager(this, 5222, m_configuration);
    m_boshManager = new BoshManager(this, 5280, 5222);

//    connect(m_boshManager, SIGNAL(sigBoshSessionInitiation(QString,QString)), m_conManager,
//            SLOT(boshSessionInitiation(QString,QString)));
//    connect(m_boshManager, SIGNAL(sigBoshSessionRequests(QString,QString,QString,QList<QDomDocument>)),
//            m_conManager, SLOT(boshSessionRequest(QString,QString,QString,QList<QDomDocument>)));
//    connect(m_conManager, SIGNAL(sigConnectionManagerBoshSessionInitiationReply(QString,QDomDocument)),
//            m_boshManager, SLOT(boshSessionInitiationReply(QString,QDomDocument)));
//    connect(m_conManager, SIGNAL(sigConnectionManagerBoshRequestReply(QString,QList<QDomDocument>)), m_boshManager,
//            SLOT(boshSessionRequestReply(QString,QList<QDomDocument>)));
}

void Server::start()
{
    qDebug() << "Hello welcome to QJabberd XMPP server version 0.1";
    m_conManager->startManage();
    m_boshManager->start();
}

void Server::stop()
{
    m_conManager->stopManage();
}

void Server::restart()
{
    m_conManager->stopManage();
    m_conManager->startManage();
}
