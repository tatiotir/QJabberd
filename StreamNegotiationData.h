#ifndef STREAMNEGOTIATIONDATA_H
#define STREAMNEGOTIATIONDATA_H

#include <QString>
#include <QMultiHash>

class StreamNegotiationData
{
public:
    StreamNegotiationData();

    bool firstFeatureProceed();
    bool secondFeatureProceed();
    bool bindFeatureProceed();
    bool secondChallengeGenerated();
    QString getHost();
    QMultiHash<QString, QByteArray> getClientFirstResponseDataMap();

    void setFirstFeatureProceed(bool val);
    void setSecondFeatureProceed(bool val);
    void setBindFeatureProceed(bool val);
    void setSecondChallengeGenerated(bool val);
    void setHost(QString host);
    void setClientFirstResponseDataMap(QMultiHash<QString, QByteArray> dataMap);

    QString mechanism() const;
    void setMechanism(const QString &mechanism);

    QString nonce() const;
    void setNonce(const QString &nonce);

private:
    bool m_firstFeatureProceed;
    bool m_secondFeatureProceed;
    bool m_bindFeatureProceed;
    bool m_secondChallengeGenerated;
    QString m_host;
    QString m_mechanism;
    QString m_nonce;
    QMultiHash<QString, QByteArray> m_clientFirstResponseDataMap;
};

#endif // STREAMNEGOTIATIONDATA_H
