#ifndef STREAMNEGOTIATION_H
#define STREAMNEGOTIATION_H

#include <QObject>
#include <QDomElement>
#include <QCryptographicHash>
#include <QDebug>
#include "sasl.h"

class StreamNegotiation
{
public:
    StreamNegotiation();

    QByteArray reply(QByteArray clientXML);
    QByteArray tlsFeature();
    QByteArray saslFeature();
    QByteArray bindFeature();
    bool tlsFeatureProceed();
    bool saslFeatureProceed();
    bool bindFeatureProceed();
    QByteArray generateFirstStreamReply(QByteArray clientXml);
    QByteArray generateStartTlsReply();
    QByteArray generateFirstChallengeReply(QByteArray clientXml);
    QByteArray generateSecondChallengeReply();
    QByteArray generateSaslSuccessReply();
    void tlsNegotiationSuccess();
    QString getUserJid();

private:
    bool parse(QByteArray decodeResponse);
    bool m_tlsFeatureProceed;
    bool m_saslFeatureProceed;
    bool m_bindFeatureProceed;
    QString m_host;
    QMultiHash<QString, QByteArray> m_map;
};

#endif // STREAMNEGOTIATION_H
