#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "UserManager.h"
#include "PrivacyListManager.h"
#include "MucManager.h"
#include "Error.h"
#include "DataFormManager.h"
#include "BlockingCommandManager.h"

class MessageManager : public QObject
{
    Q_OBJECT
public:
    MessageManager(QObject *parent = 0, QJsonObject *serverConfiguration = 0, UserManager *userManager = 0,
                   PrivacyListManager *privacyListManager = 0, MucManager *mucManager = 0,
                   BlockingCommandManager *blockingCmdManager = 0);
    QByteArray parseMessage(QDomDocument document, QString messageFrom);

public slots:

signals:
    void sigNewChatMessage(QString to, QDomDocument document);
    void sigGroupchatMessage(QString to, QDomDocument document);
    void sigDirectMucInvitation(QString to, QDomDocument document);
    void sigMucPresenceBroadCast(QString to, QDomDocument document);
    void sigApplicationRequest(QString to, QDomDocument document);

private:
    QDomDocument generatePubsubSubscriptionApprovalNotification(QString from, QString to, QString id, QString node,
                                                              QString jid, QString subscription);

    UserManager *m_userManager;
    MucManager *m_mucManager;
    PrivacyListManager *m_privacyListManager;
    BlockingCommandManager *m_blockingCmdManager;
    QJsonObject *m_serverConfiguration;
};

#endif // MESSAGEMANAGER_H
