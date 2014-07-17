#include "Sasl.h"

Sasl::Sasl()
{

}

QByteArray Sasl::generateNonce()
{
    QByteArray data = "qjabberd" + QTime::currentTime().toString().toUtf8() + "tatiotimbogningraoul";
    return data.toBase64();
}

QByteArray Sasl::calculateDigest(QByteArray method, QMultiHash<QString, QByteArray> map)
{
    QByteArray A1;
    QByteArray firstPart = map.value("username") + ":" + map.value("realm") + ":" + map.value("password");
    QByteArray firstHex = QCryptographicHash::hash(firstPart, QCryptographicHash::Md5);

    if (!map.value("authzid").isEmpty())
    {
        QByteArray secondPart = ":" + map.value("nonce") + ":" + map.value("cnonce") + ":" + map.value("authzid");
        A1 = firstHex + secondPart;
    }
    else
    {
        QByteArray secondPart = ":" + map.value("nonce") + ":" + map.value("cnonce");
        A1 = firstHex + secondPart;
    }

    QByteArray A2;
    if (map.value("qop") == "auth")
    {
        A2 = method + map.value("digest-uri");
    }
    else if ((map.value("qop") == "auth-int") || (map.value("auth") == "auth-con"))
    {
        A2 = method + map.value("digest-uri") + ":00000000000000000000000000000000";
    }

    QByteArray hexA1 = QCryptographicHash::hash(A1, QCryptographicHash::Md5).toHex();
    QByteArray hexA2 = QCryptographicHash::hash(A2, QCryptographicHash::Md5).toHex();

    QByteArray second = map.value("nonce") + ":" + map.value("nc") + ":" + map.value("cnonce") + ":"
            + map.value("qop") + ":" + hexA2;

    QByteArray kd = hexA1 + ":" + second;
    QByteArray responseValue = QCryptographicHash::hash(kd, QCryptographicHash::Md5).toHex();

    return responseValue;
}

QByteArray Sasl::generateResponseValue(QMultiHash<QString, QByteArray> map)
{
    if (calculateDigest("AUTHENTICATE:", map) == map.value("response"))
    {
        return calculateDigest(":", map);
    }
    else
    {
        return QByteArray("error");
    }
}

QMultiHash<QString, QByteArray> Sasl::decode(QByteArray message, QString mechanism)
{
    QMultiHash<QString, QByteArray> clientResponseDataMap;
    if (mechanism == "DIGEST-MD5")
    {
        QList<QByteArray> m_clientFirstResponseMapList = message.split(',');
        foreach (QByteArray data, m_clientFirstResponseMapList)
        {
            QString key = QString(data.left(data.indexOf('=')));
            QByteArray value = data.mid(data.indexOf('=') + 1).replace(QByteArray("\""), QByteArray(""));
            clientResponseDataMap.insert(key, value);
        }
    }
    else if (mechanism == "SCRAM-SHA-1")
    {
        if ((message.at(0) == 'n') || (message.at(0) == 'y') || (message.at(0) == 'p'))
        {
            message.remove(0, 3);
        }
        QList<QByteArray> m_clientFirstResponseMapList = message.split(',');
        foreach (QByteArray data, m_clientFirstResponseMapList)
        {
            QString key = QString(data.left(data.indexOf('=')));
            QByteArray value = data.mid(data.indexOf('=') + 1).replace(QByteArray("\""), QByteArray(""));
            clientResponseDataMap.insert(key, value);
        }
    }
    return clientResponseDataMap;
}

QString Sasl::generateSalt(QString str, int count)
{
    QString u0 = QByteArray("\0\0\0\1");
    QString u1 = Utils::hmac(str.toUtf8(), u0.toUtf8());
    QString u2;
    QString salt = u1;
    for (int i = 0; i < count; ++i)
    {
        u2 = Utils::hmac(str.toUtf8(), u1.toUtf8());
        salt = Utils::XOR(salt.toUtf8(), u2.toUtf8());
        u1 = u2;
    }
    return salt;
}
