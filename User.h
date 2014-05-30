#ifndef USER_H
#define USER_H

#include <QString>
#include <QTime>
#include "Utils.h"
#include "Stream.h"
#include "Connection.h"

class User
{
public:

    User(Stream *stream = 0, QString smId = QString(), int presencePriority = 0,
         int inboundStanzaCount = 0, QByteArray currentPresence = QByteArray());
    //User();

    Stream *getStream();
    QString getSmId();
    QByteArray getCurrentPresence();
    int getConnectedTime();
    int getPresencePriority();
    int getInboundStanzaReceivedCount();

    void setSmId(QString smId);
    void setCurrentPresence(QByteArray currentPresence);
    void setInboundStanzaReceivedCount(int inboundStanzaReceivedCount);
    void setPresencePriority(int presencePriority);
    void setStream(Stream *stream);
    
private:
    int m_presencePriority;
    int m_inboundStanzaReceivedCount; // For stream management
    Stream *m_stream;
    QTime m_connectedTime;
    QByteArray m_currentPresence;
    QString m_smId;
};

bool operator <(User user1, User user2);

#endif // USER_H
