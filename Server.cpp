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
        m_configMap = new QMap<QString, QVariant>(configDocument.toVariant().toMap());
        configFile.close();
    }
}

void Server::start()
{
    m_conManager = new ConnectionManager(this, 5222, m_configMap);
    m_conManager->startManage();
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
