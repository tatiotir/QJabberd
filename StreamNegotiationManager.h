#ifndef STREAMNEGOTIATIONMANAGER_H
#define STREAMNEGOTIATIONMANAGER_H

#include <QObject>
#include <QDomElement>
#include <QDomDocument>
#include <QCryptographicHash>
#include <QDebug>
#include "Error.h"
#include "Sasl.h"
#include "UserManager.h"
#include "StreamNegotiationData.h"

class StreamNegotiationManager : public QObject
{
    Q_OBJECT
public:
    StreamNegotiationManager(QObject *parent = 0, QJsonObject *serverConfiguration = 0,
                             UserManager *userManager = 0);

public slots:
    QByteArray reply(QDomDocument document, QString streamId);
    QByteArray firstFeatures();
    QByteArray secondFeatures();
    QByteArray bindFeature();
    bool secondFeatureProceed(QString streamId);
    bool firstFeatureProceed(QString streamId);
    bool bindFeatureProceed(QString streamId);
    QByteArray generateFirstStreamReply(QDomDocument document, QString streamId);
    QByteArray generateStartTlsReply();
    QByteArray generateFirstChallengeReply(QDomDocument document, QString streamId);
    QByteArray generateSecondChallengeReply(QString mechanism, QString streamId);
    QByteArray generateSaslSuccessReply(QString streamId);
    void firstNegotiationSuccess(QString streamId);
    QString getUserJid(QString streamId);
    void resourceBind(QString streamId);

    void setStreamNegotiationData(QString streamId, StreamNegotiationData *strData);

signals:
    void sigStreamNegotiationError(QString streamId);
    void sigHost(QString streamId, QString host);

private:
    bool parse(QByteArray clientResponse, QString streamId);

    UserManager *m_userManager;
    QMultiHash<QString, StreamNegotiationData* > *m_streamNegotiationVariableMap;
    QJsonObject *m_serverConfiguration;
};

#endif // STREAMNEGOTIATIONMANAGER_H
