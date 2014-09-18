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

    if (m_configuration->value("modules").toObject().value("bosh").toBool())
        m_boshManager = new BoshManager(this, 5280, 5222, m_configuration->value("crossDomainBosh").toBool());
}

void Server::start()
{
    qDebug() << "Hello welcome to QJabberd XMPP server version 0.1";
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
