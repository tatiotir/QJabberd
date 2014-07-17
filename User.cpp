#include "User.h"

User::User(Stream *stream, QString smId, int presencePriority, int inboundStanzaCount,
           QByteArray currentPresence)
{
    m_stream = stream;
    m_inboundStanzaReceivedCount = inboundStanzaCount;
    m_presencePriority = presencePriority;
    m_currentPresence = currentPresence;
    m_smId = smId;

    m_connectedTime = QTime();
    m_connectedTime.start();
}

//User::User()
//{
//    m_stream = new Stream();
//}

Stream* User::getStream()
{
    return m_stream;
}

QString User::getSmId()
{
    return m_smId;
}

QByteArray User::getCurrentPresence()
{
    return m_currentPresence;
}

int User::getConnectedTime()
{
    return m_connectedTime.elapsed();
}

int User::getInboundStanzaReceivedCount()
{
    return m_inboundStanzaReceivedCount;
}

int User::getPresencePriority()
{
    return m_presencePriority;
}

void User::setStream(Stream *stream)
{
    m_stream = stream;
}

void User::setSmId(QString smId)
{
    m_smId = smId;
}

void User::setCurrentPresence(QByteArray currentPresence)
{
    m_currentPresence = currentPresence;
}

void User::setInboundStanzaReceivedCount(int inboundStanzaReceivedCount)
{
    m_inboundStanzaReceivedCount = inboundStanzaReceivedCount;
}

void User::setPresencePriority(int presencePriority)
{
    m_presencePriority = presencePriority;
}

bool operator <(User user1, User user2)
{
    return (user1.getPresencePriority() < user2.getPresencePriority() ? true : false);
}
