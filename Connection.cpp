#include "Connection.h"

Connection::Connection(QObject *parent) :
    QSslSocket(parent)
{

}

Connection::Connection() : QSslSocket()
{

}
