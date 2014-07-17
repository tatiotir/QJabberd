#ifndef BOSHSESSION_H
#define BOSHSESSION_H

#include <QObject>
#include <QTcpSocket>
#include <QDomDocument>
#include <QJsonObject>
#include <QTimer>
#include <QThread>
#include <QHostInfo>
#include <QQueue>
#include "Utils.h"
#include "Connection.h"

class BoshSession : public QThread
{
    Q_OBJECT
public:
    static const QString m_version;
    static const QString m_charsets;
    static const int m_inactivity;
    static const int m_polling;
    static const int m_request;
    static const int m_maxpause;

    BoshSession(QObject *parent = 0, Connection *xmppServerConnection = 0, QString content = QString(),
                QString from = QString(),
                int hold = -1, int rid = -1, QString host = QString(), QString route = QString(),
                int wait = -1, int ack = -1,  QString xmlLang = QString(), QString sid = QString());

    QString content() const;
    void setContent(const QString &content);

    QString from() const;
    void setFrom(const QString &from);

    int hold() const;
    void setHold(int hold);

    int rid() const;
    void setRid(int rid);

    QString host() const;
    void setHost(const QString &host);

    QString route() const;
    void setRoute(const QString &route);

    int wait() const;
    void setWait(int wait);

    int ack() const;
    void setAck(int ack);

    QString xmlLang() const;
    void setXmlLang(const QString &xmlLang);

    QString sid() const;
    void setSid(const QString &sid);

    QString fullJid() const;
    void setFullJid(const QString &fullJid);

    Connection *xmppServerConnection() const;
    void setXmppServerConnection(Connection *xmppServerConnection);

    int nbRequest() const;
    void setNbRequest(int nbRequest);

    Connection *boshFirstConnection() const;
    void setBoshFirstConnection(Connection *boshFirstConnection);

    Connection *boshSecondConnection() const;
    void setBoshSecondConnection(Connection *boshSecondConnection);

    int activeConnection() const;
    void setActiveConnection(int activeConnection);

signals:
    void sigCloseBoshSession(QString sid);

public slots:
    void boshSessionReply();
    void boshSessionRequestReply();
    void boshSessionInitiationReply(QDomDocument document);
    void boshSessionRestartReply(QDomDocument document);
    void sessionRequest(QDomDocument request);
    void sendRequest();
    void sendKeepAlive();
    void requestReply(QByteArray reply);
    void emptyRequestReply(QByteArray reply);
    void initXmppServerStream();
    void xmppServerDataReceived();
    void close();

protected:
    void run();

private:
    Connection *m_boshFirstConnection;
    Connection *m_boshSecondConnection;
    Connection *m_xmppServerConnection;
    QString m_content;
    QString m_from;
    int m_hold;
    int m_rid;
    QString m_host;
    QString m_route;
    int m_wait;
    int m_ack;
    QString m_xmlLang;
    QString m_sid;
    QString m_fullJid;
    QTimer *m_keepAliveTimer;
    QTimer *m_emptyRequestTimer;
    QByteArray m_xmlPaquet;
    bool m_saslNegotiated;
    int m_nbRequest;
    int m_activeConnection;
    QList<QByteArray> m_xmppServerResponseList;
    QQueue<QDomDocument> m_xmppServerRequestQueue;
};

#endif // BOSHSESSION_H
