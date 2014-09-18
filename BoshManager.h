#ifndef BOSHMANAGER_H
#define BOSHMANAGER_H

#include "BoshSessionManager.h"

class BoshManager : public QObject
{
    Q_OBJECT
public:
    BoshManager(QObject *parent = 0, int boshPort = 5280, int xmppServerPort = 5222,
                bool crossDomainBosh = false);

    void start();
    void stop();
    void restart();

public slots:

signals:
    void sigBoshSessionRequests(QString sid, QString fullJid, QString host, QList<QDomDocument> requests);
    void sigBoshSessionInitiation(QString sid, QString host);

private:
    BoshSessionManager *m_boshSessionManager;
};

#endif // BOSHMANAGER_H
