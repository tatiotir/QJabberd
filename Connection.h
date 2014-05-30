#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSslSocket>

/*!
 * \class Connection
 *
 * \brief The Connection class represent the client connection. It subclass the QSslSocket class.
 */
class Connection : public QSslSocket
{
    Q_OBJECT
public:
    Connection();
    Connection(QObject *parent = 0);

signals:

public slots:

};

#endif // CONNECTION_H
