#include "Sasl.h"

Sasl::Sasl()
{

}

QByteArray Sasl::generateNonce()
{
    QByteArray data = "qjabberd" + QTime::currentTime().toString().toUtf8() + "tatiotiraoulsowasamo";
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
