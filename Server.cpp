#include "Server.h"

Server::Server()
{
    QFile *configFile = new QFile("qjabberd.config");
    if (!configFile->open(QIODevice::ReadOnly))
    {
        qDebug() << "Error in the server configuration file";
    }
    else
    {
        QJsonDocument configDocument = QJsonDocument::fromJson(configFile->readAll());
        m_configuration = new QJsonObject(configDocument.object());
        configFile->close();
    }

    m_conManager = new ConnectionManager(this, m_configuration->value("xmppServerPort").toInt(),
                                         m_configuration);

    if (m_configuration->value("modules").toObject().value("bosh").toBool())
        m_boshManager = new BoshManager(this, m_configuration->value("Bosh").toObject().value("port").toInt(),
                                        m_conManager->getPort(), m_configuration->value("Bosh").toObject().value("crossDomainBosh").toBool());
}

void Server::start()
{
    qDebug() << "Hello welcome to QJabberd XMPP server version 1.0";
    m_conManager->startManage();

    if (m_configuration->value("modules").toObject().value("bosh").toBool())
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
