#ifndef PRESENCEMANAGER_H
#define PRESENCEMANAGER_H

#include <QString>
#include <QDomElement>
#include "UserManager.h"
#include "RosterManager.h"
#include "LastActivityManager.h"
#include "PrivacyListManager.h"

class PresenceManager : public QObject
{
    Q_OBJECT
public:
    PresenceManager(UserManager *usermanager = 0, RosterManager *rosterManager = 0,
                    LastActivityManager *lastActivityManager = 0, PrivacyListManager *privateListManager = 0);

    static QDomDocument generatePresence(QString type, QString from, QString to, QString id, QString show,
                                       QString priority, QMultiHash<QString, QString> status);
    static QDomDocument generatePresence(QString type, QString from, QString to, QString id, QString show,
                                       QString priority, QString status);
public slots:
    QByteArray parsePresence(QDomDocument document, QString presenceFrom);
    void presenceUnavailableBroadcast(QString to, QString from);
    void presenceBroadcast(QString to, QDomDocument document);
    void deleteOfflinePresenceSubscribe(QString from, QString to);

signals:
    void sigRosterPush(QString to, QDomDocument document);
    void sigPresenceBroadCast(QString to, QDomDocument document);
    void sigPresenceBroadCastFromContact(QString to, QString contactJid);
    //void sigPresenceProbeReply(QString from, QString to, QString id, QByteArray data, bool contactSubscribedToUser);
    void sigPresenceProbeToContact(QString to, QString from, bool directedPresenceProbe);
    void sigPresencePriority(QString fullJid, int priority);
    void sigPresenceUnavailableBroadCast(QString to, QString from);
    void sigCurrentPresence(QString fullJid, QByteArray presenceStanza);
    void sigDirectedPresence(QString from, QString to, QByteArray data);
    void sigChatRoomDirectedPresence(QString from, QString to);

private:
    PrivacyListManager *m_privacyListManager;
    RosterManager *m_rosterManager;
    LastActivityManager *m_lastActivityManager;
    QList<QString> m_directedPresenceList;
    UserManager *m_userManager;
    bool m_clientSendFirstPresence;
};

#endif // PRESENCEMANAGER_H
