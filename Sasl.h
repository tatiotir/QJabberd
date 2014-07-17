#ifndef SASL_H
#define SASL_H

#include <QTime>
#include <QStringList>
#include <QCryptographicHash>
#include <QDebug>
#include <QByteArray>
#include "Utils.h"

class Sasl
{
public:
    Sasl();

    static QByteArray generateNonce();
    static QByteArray generateResponseValue(QMultiHash<QString, QByteArray> map);
    static QByteArray calculateDigest(QByteArray method, QMultiHash<QString, QByteArray> map);
    static QMultiHash<QString, QByteArray> decode(QByteArray message, QString mechanism);
    static QString generateSalt(QString str, int count);
};

#endif // SASL_H
