#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "usermanager.h"
#include "PrivacyListManager.h"
#include "Error.h"

class MessageManager : public QObject
{
    Q_OBJECT
public:
    MessageManager(UserManager *userManager = 0, PrivacyListManager *privacyListManager = 0);
    QByteArray parseMessage(QDomDocument document, QString messageFrom);

public slots:

signals:
    void sigNewChatMessage(QString to, QDomDocument document);

private:
    UserManager *m_userManager;
    PrivacyListManager *m_privacyListManager;
};

#endif // MESSAGEMANAGER_H
