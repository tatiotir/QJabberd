#ifndef LASTACTIVITYMANAGER_H
#define LASTACTIVITYMANAGER_H

#include <QObject>
#include "UserManager.h"
#include "RosterManager.h"
#include "Error.h"

class LastActivityManager : public QObject
{
    Q_OBJECT
public:
    explicit LastActivityManager(QObject *parent = 0, UserManager *userManager = 0, RosterManager *rosterManager = 0,
                                 StorageManager *storageManager = 0);
    QByteArray lastActivityReply(QDomDocument document, QString iqFrom);
    QByteArray lastActivityQueryResult(QString from, QString to, QString id, int seconds, QString lastStatus);

    void setLastLogoutTime(QString jid, QString lastLogoutTime);
    QString getLastLogoutTime(QString jid);
    void setLastStatus(QString jid, QString status);
    QString getLastStatus(QString jid);

signals:
    void sigLastActivityQuery(QString from, QString to, QString id, QString lastStatus);
    void sigServerLastActivityQuery(QString from, QString to, QString id);
    void sigSendReceiptRequest(QString to, QByteArray data);

public slots:

private:
    UserManager *m_userManager;
    RosterManager *m_rosterManager;
    StorageManager *m_storageManager;

};

#endif // LASTACTIVITYMANAGER_H
