#ifndef PRESENCEMANAGER_H
#define PRESENCEMANAGER_H

#include <QString>
#include <QDomElement>
#include "UserManager.h"
#include "RosterManager.h"
#include "LastActivityManager.h"
#include "PrivacyListManager.h"
#include "MucManager.h"
#include "BlockingCommandManager.h"

class PresenceManager : public QObject
{
    Q_OBJECT
public:
    PresenceManager(QObject *parent = 0, UserManager *usermanager = 0, RosterManager *rosterManager = 0,
                    LastActivityManager *lastActivityManager = 0, PrivacyListManager *privateListManager = 0,
                    MucManager *mucManager = 0, BlockingCommandManager *blockingCmdManager = 0);

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
    void sigMucPresenceBroadCast(QString to, QDomDocument document);
    void sigRoomHistory(QString to, QList<QDomDocument> messageList);
    void sigRoomSubject(QString to, QByteArray subjectMessage);

private:
    QByteArray generateRoomSubjectMessage(QString from, QString to, QString id, QString subject);
    PrivacyListManager *m_privacyListManager;
    RosterManager *m_rosterManager;
    MucManager *m_mucManager;
    LastActivityManager *m_lastActivityManager;
    QList<QString> m_directedPresenceList;
    UserManager *m_userManager;
    BlockingCommandManager *m_blockingCmdManager;
    bool m_clientSendFirstPresence;
};

#endif // PRESENCEMANAGER_H
