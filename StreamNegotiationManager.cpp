/*
 * Author : Tatioti Mbogning Raoul
 * Description : This class implement TLS - SASL stream negotiation mechanism.
 */


#include "StreamNegotiationManager.h"

StreamNegotiationManager::StreamNegotiationManager(QJsonObject *serverConfiguration, UserManager *userManager)
{
    m_serverConfiguration = serverConfiguration;
    m_userManager = userManager;
    m_streamNegotiationVariableMap = new QMultiHash<QString, StreamNegotiationData* >();
}

void StreamNegotiationManager::resourceBind(QString streamId)
{
    // Delete stream negotiation data
    m_streamNegotiationVariableMap->value(streamId)->setClientFirstResponseDataMap(QMultiHash<QString, QByteArray>());
    m_streamNegotiationVariableMap->value(streamId)->setHost("");
    m_streamNegotiationVariableMap->value(streamId)->setMechanism("");
    m_streamNegotiationVariableMap->value(streamId)->setNonce("");
    m_streamNegotiationVariableMap->value(streamId)->setBindFeatureProceed(true);
}

QByteArray StreamNegotiationManager::reply(QDomDocument document, QString streamId)
{
    QString xmlTagname = document.documentElement().tagName();
    if ((xmlTagname == "iq") || (xmlTagname == "message") || (xmlTagname == "presence"))
    {
        emit sigStreamNegotiationError(streamId);
        return Error::generateStreamError("not-authorized");
    }

    if (document.documentElement().tagName() == "stream:stream")
    {
        if (!m_serverConfiguration->value("virtualHost").toVariant().toStringList().contains(document.documentElement().attribute("to")))
        {
            qDebug() << "Client disconnected. This server does not serve " << document.documentElement().attribute("to");
            emit sigStreamNegotiationError(streamId);
            return Error::generateStreamError("host-unknown");
        }

        if (document.documentElement().attribute("xmlns:stream") != "http://etherx.jabber.org/streams")
        {
            emit sigStreamNegotiationError(streamId);
            return Error::generateStreamError("invalid-namespace");
        }

        if (m_streamNegotiationVariableMap->value(streamId, new StreamNegotiationData())->getHost().isEmpty())
        {
            m_streamNegotiationVariableMap->insert(streamId, new StreamNegotiationData());
        }
        return generateFirstStreamReply(document, streamId);
    }
//    else if (document.documentElement().tagName() == "compress")
//    {
//        QString method = document.firstChildElement().firstChildElement().text();
//        if (method != "zlib")
//        {
//            return Error::generateFailureError("http://jabber.org/protocol/compress", "unsupported-method");
//        }
//        else
//        {
//            QDomDocument document;
//            QDomElement compressedElement = document.createElement("compressed");
//            compressedElement.setAttribute("xmlns", "http://jabber.org/protocol/compress");
//            document.appendChild(compressedElement);

//            return document.toByteArray();
//        }
//    }
    else if (document.documentElement().tagName() == "starttls")
    {
        return generateStartTlsReply();
    }
    else if (document.documentElement().tagName() == "auth")
    {
        QString mechanism = document.documentElement().attribute("mechanism", "");
        if (mechanism.isEmpty() || ((mechanism != "DIGEST-MD5") && (mechanism != "PLAIN")
                                    && (mechanism != "SCRAM-SHA-1")))
        {
            emit sigStreamNegotiationError(streamId);
            return Error::generateFailureError("urn:ietf:params:xml:ns:xmpp-sasl", "invalid-mechanism");
        }

        m_streamNegotiationVariableMap->value(streamId)->setMechanism(mechanism);
        if (mechanism == "PLAIN")
        {
            QStringList userInfo = QString(QByteArray::fromBase64(document.documentElement().text().toUtf8()).toPercentEncoding("@")).split("%00", QString::SkipEmptyParts);

            QString jid = userInfo.value(userInfo.count() - 2) + "@" + m_streamNegotiationVariableMap->value(streamId)->getHost();
            QString password = userInfo.value(userInfo.count() - 1);
            if (m_userManager->getPassword(jid) == password)
            {
                QMultiHash<QString, QByteArray> userMap;
                userMap.insert("username", userInfo.value(userInfo.count() - 2).toUtf8());
                m_streamNegotiationVariableMap->value(streamId)->setClientFirstResponseDataMap(userMap);
                return generateSaslSuccessReply(streamId);
            }
            else
            {
                emit sigStreamNegotiationError(streamId);
                return QByteArray();
            }
        }
        else if (mechanism == "SCRAM-SHA-1")
        {
            QByteArray message = QByteArray::fromBase64(document.documentElement().text().toUtf8());
            if ((message.at(0) != 'n') && (message.at(0) != 'y') && (message.at(0) != 'p'))
            {
                emit sigStreamNegotiationError(streamId);
                return QByteArray();
            }
            else
            {
                return generateFirstChallengeReply(document, streamId);
            }
        }
        else if (mechanism == "DIGEST-MD5")
        {
            return generateFirstChallengeReply(document, streamId);
        }
    }
    else if (document.documentElement().tagName() == "abort")
    {
        emit sigStreamNegotiationError(streamId);
        return Error::generateFailureError("urn:ietf:params:xml:ns:xmpp-sasl", "aborted");
    }
    else if (document.documentElement().tagName() == "response")
    {
        QString clientResponseValue = document.documentElement().text();
        if (!clientResponseValue.isEmpty())
        {
            // If client sended a good response
            if (this->parse(QByteArray::fromBase64(clientResponseValue.toUtf8()), streamId)
                    && !m_streamNegotiationVariableMap->value(streamId)->secondChallengeGenerated())
            {
                m_streamNegotiationVariableMap->value(streamId)->setSecondChallengeGenerated(true);
                return generateSecondChallengeReply(m_streamNegotiationVariableMap->value(streamId)->mechanism(), streamId);
            }
            else
            {
                emit sigStreamNegotiationError(streamId);
                return Error::generateFailureError("urn:ietf:params:xml:ns:xmpp-sasl", "account-disabled");
            }
        }
        else
        {
            if (m_streamNegotiationVariableMap->value(streamId)->secondChallengeGenerated())
            {
                return generateSaslSuccessReply(streamId);
            }
            else
            {
                emit sigStreamNegotiationError(streamId);
                return QByteArray();
            }
        }
    }
    return QByteArray();
}

QByteArray StreamNegotiationManager::firstFeatures()
{
    QDomDocument document;
    QDomElement streamFeatures = document.createElement("stream:features");

    QDomElement registerElement = document.createElement("register");
    registerElement.setAttribute("xmlns", "http://jabber.org/features/iq-register");

    QDomElement starttls = document.createElement("starttls");
    starttls.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");

    if (m_serverConfiguration->value("modules").toObject().value("saslauth").toBool())
    {
        QDomElement mechanisms = document.createElement("mechanisms");
        mechanisms.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

        QDomElement firstMechanism = document.createElement("mechanism");
        firstMechanism.appendChild(document.createTextNode("DIGEST-MD5"));

        QDomElement secondMechanism = document.createElement("mechanism");
        secondMechanism.appendChild(document.createTextNode("PLAIN"));

//        QDomElement thirdMechanism = document.createElement("mechanism");
//        thirdMechanism.appendChild(document.createTextNode("SCRAM-SHA-1"));

        mechanisms.appendChild(firstMechanism);
        mechanisms.appendChild(secondMechanism);
//        mechanisms.appendChild(thirdMechanism);

        streamFeatures.appendChild(mechanisms);
    }

//    if (m_serverConfiguration->value("modules").toObject().value("compress").toBool())
//    {
//        QDomElement compressionElement = document.createElement("compression");
//        compressionElement.setAttribute("xmlns", "http://jabber.org/features/compress");

//        QDomElement methodElement = document.createElement("method");
//        methodElement.appendChild(document.createElement("zlib"));

//        compressionElement.appendChild(methodElement);
//        streamFeatures.appendChild(compressionElement);
//    }

    QDomElement sub = document.createElement("sub");
    sub.setAttribute("xmlns", "urn:xmpp:features:pre-approval");

    streamFeatures.appendChild(registerElement);
    streamFeatures.appendChild(sub);
    streamFeatures.appendChild(starttls);

    document.appendChild(streamFeatures);
    return document.toByteArray();
}

QByteArray StreamNegotiationManager::secondFeatures()
{
    QDomDocument document;

    QDomElement registerElement = document.createElement("register");
    registerElement.setAttribute("xmlns", "http://jabber.org/features/iq-register");

    QDomElement authElement = document.createElement("auth");
    authElement.setAttribute("xmlns", "http://jabber.org/features/iq-auth");

    QDomElement streamFeatures = document.createElement("stream:features");

    QDomElement mechanisms = document.createElement("mechanisms");
    mechanisms.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

    QDomElement firstMechanism = document.createElement("mechanism");
    firstMechanism.appendChild(document.createTextNode("DIGEST-MD5"));

    QDomElement secondMechanism = document.createElement("mechanism");
    secondMechanism.appendChild(document.createTextNode("PLAIN"));

//    QDomElement thirdMechanism = document.createElement("mechanism");
//    thirdMechanism.appendChild(document.createTextNode("SCRAM-SHA-1"));

    mechanisms.appendChild(firstMechanism);
    mechanisms.appendChild(secondMechanism);
//    mechanisms.appendChild(thirdMechanism);

    streamFeatures.appendChild(authElement);
    streamFeatures.appendChild(registerElement);
    streamFeatures.appendChild(mechanisms);
    document.appendChild(streamFeatures);

    return document.toByteArray();
}

QByteArray StreamNegotiationManager::bindFeature()
{
    QDomDocument document;

    QDomElement streamFeatures = document.createElement("stream:features");

    QDomElement bind = document.createElement("bind");
    bind.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-bind");
    bind.appendChild(document.createElement("required"));

    QDomElement session = document.createElement("session");
    session.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-session");
    session.appendChild(document.createElement("optional"));

    QDomElement sm = document.createElement("sm");
    sm.setAttribute("xmlns", "urn:xmpp:sm:3");

    QDomElement sub = document.createElement("sub");
    sub.setAttribute("xmlns", "urn:xmpp:features:pre-approval");

    streamFeatures.appendChild(bind);
    streamFeatures.appendChild(session);
    streamFeatures.appendChild(sm);
    streamFeatures.appendChild(sub);
    document.appendChild(streamFeatures);

    return document.toByteArray();
}

bool StreamNegotiationManager::bindFeatureProceed(QString streamId)
{
    return m_streamNegotiationVariableMap->value(streamId, new StreamNegotiationData())->bindFeatureProceed();
}

bool StreamNegotiationManager::firstFeatureProceed(QString streamId)
{
    return m_streamNegotiationVariableMap->value(streamId, new StreamNegotiationData())->firstFeatureProceed();
}

bool StreamNegotiationManager::secondFeatureProceed(QString streamId)
{
    return m_streamNegotiationVariableMap->value(streamId, new StreamNegotiationData())->secondFeatureProceed();
}

QByteArray StreamNegotiationManager::generateFirstStreamReply(QDomDocument document, QString streamId)
{
    QDomElement xmlElement = document.documentElement();

    QString from = xmlElement.attribute("from", "");
    QString to = xmlElement.attribute("to");

    m_streamNegotiationVariableMap->value(streamId)->setHost(to);
    emit sigHost(streamId, to);

    QString xmlVersion = xmlElement.attribute("version", "1.0");
    QString xmlLang = xmlElement.attribute("xml:lang", "en");
    QString xmlns = xmlElement.attribute("xmlns");
    QString xmlnsStream = xmlElement.attribute("xmlns:stream");

    QDomDocument replyDocument;
    QDomElement replyElement = replyDocument.createElement("stream:stream");

    if (from != "")
    {
        replyElement.setAttribute("to", from);
    }
    replyElement.setAttribute("from", to);
    replyElement.setAttribute("version", xmlVersion);
    replyElement.setAttribute("xml:lang", xmlLang);
    replyElement.setAttribute("xmlns", xmlns);
    replyElement.setAttribute("xmlns:stream", xmlnsStream);

    replyDocument.appendChild(replyElement);

    QByteArray reply = replyDocument.toByteArray().replace(QByteArray("/>"), QByteArray(">"));
    if (!m_streamNegotiationVariableMap->value(streamId)->firstFeatureProceed()
            && !m_streamNegotiationVariableMap->value(streamId)->secondFeatureProceed())
    {
        reply += firstFeatures();
    }
    else if (!m_streamNegotiationVariableMap->value(streamId)->secondFeatureProceed())
    {
        reply += secondFeatures();
    }
    else
    {
        reply += bindFeature();
    }

    return reply;
}

QByteArray StreamNegotiationManager::generateStartTlsReply()
{
    QDomDocument document;
    QDomElement proceed = document.createElement("proceed");
    proceed.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");
    document.appendChild(proceed);

    QByteArray reply = document.toByteArray() + QByteArray("#") +
            m_serverConfiguration->value("ssl").toObject().value("certificate").toVariant().toByteArray() + QByteArray("#") +
            m_serverConfiguration->value("ssl").toObject().value("key").toVariant().toByteArray();
    return reply;
}

QByteArray StreamNegotiationManager::generateSaslSuccessReply(QString streamId)
{
    QDomDocument replyDocument;

    QDomElement success = replyDocument.createElement("success");
    success.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

    replyDocument.appendChild(success);

    // We have done with sasl negiciation feature
    m_streamNegotiationVariableMap->value(streamId)->setSecondFeatureProceed(true);

    // signal for IQManager
    //emit sigSaslNegotiated(getUserJid());
    return replyDocument.toByteArray();
}

/*
 * This function generate first server challenge according to the digest mechanism for sasl authentification
 */
QByteArray StreamNegotiationManager::generateFirstChallengeReply(QDomDocument document, QString streamId)
{
    QDomElement auth = document.documentElement();

    QString mechanism = auth.attribute("mechanism");

    QByteArray firstChallenge;
    if (mechanism == "DIGEST-MD5")
    {
        firstChallenge += "realm=";
        firstChallenge += "\"" + m_streamNegotiationVariableMap->value(streamId)->getHost() + "\"";
        firstChallenge += ",nonce=";
        firstChallenge += "\"" + Sasl::generateNonce() + "\"";
        firstChallenge += ",qop=\"auth\"";
        firstChallenge += ",charset=utf-8,algorithm=md5-sess";
    }
    else if (mechanism == "SCRAM-SHA-1")
    {
        QString nonce = Sasl::generateNonce();
        QString username = Sasl::decode(QByteArray::fromBase64(document.documentElement().text().toUtf8()), "SCRAM-SHA-1").value("n");
        QString password = m_userManager->getPassword(username + "@" + m_streamNegotiationVariableMap->value(streamId)->getHost());
        firstChallenge += "r=" + nonce + ",";
        firstChallenge += "s=" + Sasl::generateSalt(password, 4096) + ",";
        firstChallenge += "i=4096";

        m_streamNegotiationVariableMap->value(streamId)->setNonce(nonce);
    }

    QDomDocument replyDocument;

    QDomElement challenge = replyDocument.createElement("challenge");
    challenge.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
    challenge.appendChild(replyDocument.createTextNode(firstChallenge.toBase64()));
    replyDocument.appendChild(challenge);
    return replyDocument.toByteArray();
}

/*
 * This function generate first server challenge according to the digest mechanism for sasl authentification
 */
QByteArray StreamNegotiationManager::generateSecondChallengeReply(QString mechanism, QString streamId)
{
    QByteArray secondChallenge;
    if (mechanism == "DIGEST-MD5")
    {
        secondChallenge = "rspauth=" + Sasl::generateResponseValue(m_streamNegotiationVariableMap->value(streamId)->getClientFirstResponseDataMap());
    }
    else if (mechanism == "SCRAM-SHA-1")
    {

    }

    QDomDocument replyDocument;

    QDomElement challenge = replyDocument.createElement("challenge");
    challenge.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
    challenge.appendChild(replyDocument.createTextNode(secondChallenge.toBase64()));

    replyDocument.appendChild(challenge);

    return replyDocument.toByteArray();
}

bool StreamNegotiationManager::parse(QByteArray clientResponse, QString streamId)
{
    QMultiHash<QString, QByteArray> clientResponseDataMap = Sasl::decode(clientResponse, m_streamNegotiationVariableMap->value(streamId)->mechanism());

    if (m_streamNegotiationVariableMap->value(streamId)->mechanism() == "DIGEST-MD5")
    {
        if ((clientResponseDataMap.count("nonce") > 1) || (clientResponseDataMap.count("cnonce") > 1)
                || (!clientResponseDataMap.value("authzid").isEmpty())
                || (clientResponseDataMap.value("nonce-count") == "0000001"))
        {
            return false;
        }
    }
    else if (m_streamNegotiationVariableMap->value(streamId)->mechanism() == "SCRAM-SHA-1")
    {
        if (clientResponseDataMap.value("r") != m_streamNegotiationVariableMap->value(streamId)->nonce())
            return false;
    }

    QString jid = clientResponseDataMap.value("username") + "@" + m_streamNegotiationVariableMap->value(streamId)->getHost();
    QString password = m_userManager->getPassword(jid);

    if (password.isEmpty())
        return false;

    clientResponseDataMap.insert("password", password.toUtf8());
    m_streamNegotiationVariableMap->value(streamId)->setClientFirstResponseDataMap(clientResponseDataMap);
    return true;
}

void StreamNegotiationManager::firstNegotiationSuccess(QString streamId)
{
    // Client have successful negociated tls feature
    m_streamNegotiationVariableMap->value(streamId)->setFirstFeatureProceed(true);
}

QString StreamNegotiationManager::getUserJid(QString streamId)
{
    return m_streamNegotiationVariableMap->value(streamId)->getClientFirstResponseDataMap().value("username") +
            "@" + m_streamNegotiationVariableMap->value(streamId)->getHost();
}

void StreamNegotiationManager::setStreamNegotiationData(QString streamId, StreamNegotiationData *strData)
{
    m_streamNegotiationVariableMap->insert(streamId, strData);
}
