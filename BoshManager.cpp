#include "BoshManager.h"

BoshManager::BoshManager(int boshPort, int xmppServerPort)
{
    m_boshSessionManager = new BoshSessionManager(boshPort, xmppServerPort);

    connect(m_boshSessionManager, SIGNAL(sigBoshSessionInitiation(QString,QString)), this,
            SIGNAL(sigBoshSessionInitiation(QString,QString)));
    connect(m_boshSessionManager, SIGNAL(sigBoshSessionRequests(QString,QString,QString,QList<QDomDocument>)),
            this, SIGNAL(sigBoshSessionRequests(QString,QString,QString,QList<QDomDocument>)));
}

void BoshManager::start()
{
    m_boshSessionManager->startManage();
}

void BoshManager::stop()
{
    m_boshSessionManager->stopManage();
}

void BoshManager::restart()
{
    m_boshSessionManager->restart();
}
