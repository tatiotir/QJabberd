#ifndef BOSHSESSIONMANAGER_H
#define BOSHSESSIONMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QHttpPart>
#include "BoshSession.h"

class BoshSessionManager : public QTcpServer
{
    Q_OBJECT
public:
    BoshSessionManager(QObject *parent = 0, int boshPort = 5280, int xmppServerPort = 5222);

protected:
    void incomingConnection(qintptr socketDescriptor);

signals:
    void sigBoshSessionRequests(QString sid, QString fullJid, QString host, QList<QDomDocument> requests);
    void sigBoshSessionInitiation(QString sid, QString host);

public slots:
    void startManage();
    void stopManage();
    void restart();
    void dataReceived();
    void closeBoshSession(QString sid);

private:
    int m_boshPort;
    int m_xmppServerPort;
    QMap<QString, BoshSession* > *m_sessionMap;
};

#endif // BOSHSESSIONMANAGER_H
