#include "User.h"

User::User(Connection *connection, QString smId, int presencePriority, int inboundStanzaCount,
           QByteArray currentPresence)
{
    m_connection = connection;
    m_inboundStanzaReceivedCount = inboundStanzaCount;
    m_presencePriority = presencePriority;
    m_currentPresence = currentPresence;
    m_smId = smId;

    m_connectedTime = QTime();
    m_connectedTime.start();
}

User::User()
{
    m_connection = new Connection(0);
    m_inboundStanzaReceivedCount = 0;
    m_presencePriority = 0;
    m_currentPresence = QByteArray();
    m_smId = "";
    m_connectedTime = QTime();
    m_connectedTime.start();
}

Connection* User::getConnection()
{
    return m_connection;
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

void User::setConnection(Connection *connection)
{
    m_connection = connection;
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
