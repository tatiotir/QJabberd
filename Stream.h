#ifndef SESSION_H
#define SESSION_H

#include <QtXml/QtXml>
#include <QDebug>
#include <QtNetwork/QtNetwork>
#include "MessageManager.h"
#include "IqManager.h"
#include "PresenceManager.h"
#include "Connection.h"

class Stream : public QObject
{
    Q_OBJECT
public:
    explicit Stream(QString streamId = QString(),
                    QJsonObject *serverConfiguration = 0,
                    Connection *connection = 0,
                    IqManager *iqManager = 0, PresenceManager *presenceManager = 0,
                    MessageManager *messageManager = 0,
                    RosterManager *rosterManager = 0, StreamNegotiationManager *streamNegotiationManager = 0,
                    BlockingCommandManager *blockingCmdManager = 0);

    ~Stream();

private slots:
    void dataReceived();
    void sslError(QList<QSslError> errors);
    void streamEncrypted();
    void sendUnavailablePresence();
//    void sendPing();
//    void pingError();
//    void sendWhiteSpace();

public slots:
    void closeStream();
    qint64 streamReply(QByteArray answer);
    void bindFeatureNegotiated(QString fullJid);
    void setHost(QString host);
    Connection* getConnection();

signals:
    void sigCloseStream(QString fullJid);
    void sigOfflineUser(QString jid);
    void sigBindFeatureNegotiated(QString fullJid, Stream *stream);
    void sigPresenceBroadCast(QString jid, QDomDocument document);
    void sigPresenceUnavailableBroadCast(QString to, QString from);
    void sigEnableStreamManagement(QString fullJid, QString smId);
    void sigInboundStanzaReceived(QString from);
    void sigAcknowledgeReceiptServerStanza(QString from, int h);
    void sigQueryInboundStanzaReceived(QString from);

    // We send the Connection attribute because the client has not bind resource
    void sigResumeStream(Connection *connection, QString prevId, int h);

protected:
    void run();

private:
    void requestTreatment(QDomDocument document);

    QJsonObject *m_serverConfiguration;
    IqManager *m_iqManager;
    PresenceManager *m_presenceManager;
    MessageManager *m_messageManager;
    RosterManager *m_rosterManager;
    StreamNegotiationManager *m_streamNegotiationManager;
    BlockingCommandManager *m_blockingCmdManager;

    Connection *m_connection;
    QByteArray m_xmlPaquet;
    QString m_fullJid;
    QString m_host;
    QString m_streamId;
//    QTimer *m_pingTimer;
//    QTimer *m_pongTimer;
    bool m_supportPing;
    int m_pingErrorCount;
    int m_pingId;
    int m_inboundStanzaCount;
    bool m_isCompressed;
};

#endif // SESSION_H
