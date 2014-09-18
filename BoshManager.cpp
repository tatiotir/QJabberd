#include "BoshManager.h"

BoshManager::BoshManager(QObject *parent, int boshPort, int xmppServerPort, bool crossDomainBosh) : QObject(parent)
{
    m_boshSessionManager = new BoshSessionManager(this, boshPort, xmppServerPort, crossDomainBosh);

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
