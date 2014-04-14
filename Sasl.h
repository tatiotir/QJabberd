#ifndef SASL_H
#define SASL_H

#include <QTime>
#include <QStringList>
#include <QCryptographicHash>
#include <QDebug>
#include <QByteArray>

class Sasl
{
public:
    Sasl();

    static QByteArray generateNonce();
    static QByteArray generateResponseValue(QMultiHash<QString, QByteArray> map);
    static QByteArray calculateDigest(QByteArray method, QMultiHash<QString, QByteArray> map);
};

#endif // SASL_H
