#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSslSocket>

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
