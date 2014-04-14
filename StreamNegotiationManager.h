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
    StreamNegotiationManager(QMap<QString, QVariant> *serverConfigMap = 0, UserManager *userManager = 0);

    QByteArray reply(QByteArray clientXML, QString streamId);
    QByteArray firstFeatures();
    QByteArray secondFeatures();
    QByteArray bindFeature();
    bool bindFeatureProceed(QString streamId);
    QByteArray generateFirstStreamReply(QByteArray clientXml, QString streamId);
    QByteArray generateStartTlsReply();
    QByteArray generateFirstChallengeReply(QByteArray clientXml, QString streamId);
    QByteArray generateSecondChallengeReply(QString streamId);
    QByteArray generateSaslSuccessReply(QString streamId);
    void firstNegotiationSuccess(QString streamId);
    QString getUserJid(QString streamId);

signals:
    void sigNonSaslAuthentification(QString streamId, QString fullJid, QString id);
    void sigResourceBinding(QString streamId, QString fullJid, QString id);
    void sigStreamNegotiationError(QString streamId);

private:
    bool parse(QByteArray clientResponse, QString streamId);
    QByteArray authentificationFields(QString id);
    QByteArray authenticate(QString streamId, QString id, QString username, QString password, QString resource, QString digest, QString host);

    UserManager *m_userManager;
    QMultiHash<QString, StreamNegotiationData* > *m_streamNegotiationVariableMap;
    QMap<QString, QVariant> *m_serverConfigMap;
};

#endif // STREAMNEGOTIATIONMANAGER_H
