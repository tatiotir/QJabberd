#ifndef SESSION_H
#define SESSION_H

#include <QThread>
#include <QtXml/QtXml>
#include <QDebug>
#include <QtNetwork/QtNetwork>
#include "MessageManager.h"
#include "IqManager.h"
#include "PresenceManager.h"
#include "BlockingCommandManager.h"
#include "Connection.h"

class Stream : public QThread
{
    enum STREAM_ERROR
    {
        INVALID_NAMESPACE = 0,
        HOST_UNKNOWN = 1,
        BAD_NAMESPACE_PREFIX = 2,
        CONFLICT = 3,
        CONNECTION_TIMEOUT = 4,
        HOST_GONE = 5,
        INTERNAL_SERVER_ERROR = 6
    };

    Q_OBJECT
public:
    explicit Stream(QObject *parent = 0, QString streamId = QString(),
                    Connection *connection = 0,
                    IqManager *iqManager = 0, PresenceManager *presenceManager = 0,
                    MessageManager *messageManager = 0,
                    RosterManager *rosterManager = 0, StreamNegotiationManager *streamNegotiationManager = 0,
                    BlockingCommandManager *blockingCmdManager = 0);

private slots:
    void dataReceived();
    void closeStream();
    void sslError(QList<QSslError> errors);
    void streamEncrypted();
    void sendUnavailablePresence();

public slots:
    void streamReply(QByteArray answer);
    void bindFeatureNegotiated(QString fullJid);
    void setHost(QString host);
    Connection* getConnection();

signals:
    void sigOfflineUser(QString jid);
    void sigBindFeatureNegotiated(QString fullJid, Stream *stream);
    void sigPresenceBroadCast(QString jid, QByteArray data);
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

    IqManager *m_iqManager;
    PresenceManager *m_presenceManager;
    MessageManager *m_messageManager;
    RosterManager *m_rosterManager;
    StreamNegotiationManager *m_streamNegotiationManager;
    BlockingCommandManager *m_blockingCmdManager;

    Connection *m_connection;
    QByteArray m_xmlPaquet;
    QDomDocument *m_document;
    QString m_fullJid;
    QString m_host;
    QString m_streamId;
    int m_inboundStanzaCount;
};

#endif // SESSION_H
