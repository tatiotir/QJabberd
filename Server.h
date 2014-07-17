#ifndef SERVER_H
#define SERVER_H

#include "ConnectionManager.h"
#include "BoshManager.h"

class Server : public QObject
{
    Q_OBJECT
public:
    Server();

    void start();
    void stop();
    void restart();

private:
    ConnectionManager *m_conManager;
    BoshManager *m_boshManager;
    QJsonObject *m_configuration;
};

#endif // SERVER_H
