/*
 * Author : Tatioti Mbogning Raoul
 * Description : This class implement TLS - SASL stream negotiation mechanism.
 */


#include "StreamNegotiationManager.h"

StreamNegotiationManager::StreamNegotiationManager(QMap<QString, QVariant> *serverConfigMap, UserManager *userManager)
{
    m_serverConfigMap = serverConfigMap;
    m_userManager = userManager;
    m_streamNegotiationVariableMap = new QMultiHash<QString, StreamNegotiationData* >();
}

QByteArray StreamNegotiationManager::reply(QByteArray clientXML, QString streamId)
{
    QDomDocument document;
    document.setContent(clientXML);

    if (document.documentElement().tagName() == "stream:stream")
    {
        if (m_streamNegotiationVariableMap->value(streamId, new StreamNegotiationData())->getHost().isEmpty())
        {
            m_streamNegotiationVariableMap->insert(streamId, new StreamNegotiationData());
        }
        return generateFirstStreamReply(clientXML, streamId);
    }
    else if (document.documentElement().tagName() == "starttls")
    {
        return generateStartTlsReply();
    }
    else if (document.documentElement().tagName() == "auth")
    {
        QString mechanism = document.documentElement().attribute("mechanism", "");
        if (mechanism.isEmpty() /*|| ((mechanism != "PLAIN") && (mechanism != "EXTERNAL")*/
                && (mechanism != "DIGEST-MD5"))
        {
            return Error::generateSaslFailureError("invalid-mechanism");
        }
        return generateFirstChallengeReply(clientXML, streamId);
    }
    else if (document.documentElement().tagName() == "abort")
    {
        return Error::generateSaslFailureError("aborted");
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
                return generateSecondChallengeReply(streamId);
            }
            else
            {
                emit sigStreamNegotiationError(streamId);
                return Error::generateSaslFailureError("account-disabled");
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
    // Resource binding
    else if (document.documentElement().firstChildElement().tagName() == "bind")
    {
        QDomElement bindChild = document.documentElement().firstChild().toElement();
        QString fullJid;
        QString resource;

        // Empty resource binding
        if (bindChild.text().isEmpty())
        {
            resource = Utils::generateResource();
            fullJid = getUserJid(streamId) + "/" + resource;
        }
        else
        {
            resource = bindChild.firstChild().toElement().text();
            fullJid = getUserJid(streamId) + "/" + resource;
        }
        // Delete stream negotiation data
        m_streamNegotiationVariableMap->value(streamId)->setClientFirstResponseDataMap(QMultiHash<QString, QByteArray>());
        m_streamNegotiationVariableMap->value(streamId)->setHost("");
        m_streamNegotiationVariableMap->value(streamId)->setBindFeatureProceed(true);

        emit sigResourceBinding(streamId, fullJid, document.documentElement().attribute("id"));
        return QByteArray();
    }
    else if (document.documentElement().attribute("type") == "get")
    {
        if (document.documentElement().firstChildElement().attribute("xmlns") == "jabber:iq:auth")
        {
            return authentificationFields(document.documentElement().attribute("id"));
        }
    }
    else if (document.documentElement().attribute("type") == "set")
    {
        return authenticate(streamId, document.documentElement().attribute("id"),
                            document.documentElement().elementsByTagName("username").item(0).toElement().text(),
                            document.documentElement().elementsByTagName("password").item(0).toElement().text(),
                            document.documentElement().elementsByTagName("resource").item(0).toElement().text(),
                            document.documentElement().elementsByTagName("digest").item(0).toElement().text(),
                            document.documentElement().attribute("to"));
    }
}

QByteArray StreamNegotiationManager::authenticate(QString streamId, QString id, QString username, QString password,
                                     QString resource, QString digest, QString host)
{
    if (username.isEmpty() || resource.isEmpty())
    {
        return Error::generateError("iq", "modify", "not-acceptable", "", "", id, QDomElement());
    }

    if (!digest.isEmpty())
    {
        QString jid = username + '@' + host;
        QString password = m_userManager->getPassword(jid);

        QString userDigest = Utils::digestCalculator(id, password);
        if ((userDigest != digest))
        {
            Error::generateError("iq", "auth", "not-authorized", "", "", id, QDomElement());
        }
        else
        {
            emit sigNonSaslAuthentification(streamId, jid + "/" + resource, id);
            return QByteArray();
        }
    }
    else
    {
        QString jid = username + '@' + host;
        QString userPassword = m_userManager->getPassword(jid);

        if (userPassword != password)
        {
            return Error::generateError("iq", "auth", "not-authorized", "", "", id, QDomElement());
        }
        else
        {
            emit sigNonSaslAuthentification(streamId, jid + "/" + resource, id);
            return QByteArray();
        }
    }
    return QByteArray();
}


/*
 * This function send to client the fields required for authentification in the server.
 */
QByteArray StreamNegotiationManager::authentificationFields(QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");

    if (id != "")
    {
        iq.setAttribute("id", id);
    }

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:auth");

    query.appendChild(document.createElement("username"));
    query.appendChild(document.createElement("password"));
    query.appendChild(document.createElement("digest"));
    query.appendChild(document.createElement("ressouces"));

    iq.appendChild(query);
    document.appendChild(iq);

    return document.toByteArray();
}

QByteArray StreamNegotiationManager::firstFeatures()
{
    QDomDocument document;
    QDomElement streamFeatures = document.createElement("stream:features");

    QDomElement registerElement = document.createElement("register");
    registerElement.setAttribute("xmlns", "http://jabber.org/features/iq-register");

    QDomElement starttls = document.createElement("starttls");
    starttls.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");

    if (m_serverConfigMap->value("modules").toMap().value("saslauth").toBool())
    {
        QDomElement mechanisms = document.createElement("mechanisms");
        mechanisms.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

        QDomElement firstMechanism = document.createElement("mechanism");
        firstMechanism.appendChild(document.createTextNode("DIGEST-MD5"));

        /*QDomElement secondMechanism = document.createElement("mechanism");
        secondMechanism.appendChild(document.createTextNode("PLAIN"));*/

        mechanisms.appendChild(firstMechanism);
        //mechanisms.appendChild(secondMechanism);

        streamFeatures.appendChild(mechanisms);
    }

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
    authElement.setAttribute("xmlns", "http://jabber.org/features/ia-auth");

    QDomElement streamFeatures = document.createElement("stream:features");

    QDomElement mechanisms = document.createElement("mechanisms");
    mechanisms.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

    QDomElement firstMechanism = document.createElement("mechanism");
    firstMechanism.appendChild(document.createTextNode("DIGEST-MD5"));

    /*QDomElement secondMechanism = document.createElement("mechanism");
    secondMechanism.appendChild(document.createTextNode("PLAIN"));

    QDomElement thirdMechanism = document.createElement("mechanism");
    thirdMechanism.appendChild(document.createTextNode("EXTERNAL"));*/

    mechanisms.appendChild(firstMechanism);
    //mechanisms.appendChild(secondMechanism);
    //mechanisms.appendChild(thirdMechanism);

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

    QDomElement sm = document.createElement("sm");
    sm.setAttribute("xmlns", "urn:xmpp:sm:3");

    QDomElement sub = document.createElement("sub");
    sub.setAttribute("xmlns", "urn:xmpp:features:pre-approval");

    streamFeatures.appendChild(bind);
    streamFeatures.appendChild(sm);
    streamFeatures.appendChild(sub);
    document.appendChild(streamFeatures);

    return document.toByteArray();
}

bool StreamNegotiationManager::bindFeatureProceed(QString streamId)
{
    return m_streamNegotiationVariableMap->value(streamId, new StreamNegotiationData())->bindFeatureProceed();
}

QByteArray StreamNegotiationManager::generateFirstStreamReply(QByteArray clientXml, QString streamId)
{
    QDomDocument document;
    document.setContent(clientXml);

    QDomElement xmlElement = document.documentElement();

    QString from = xmlElement.attribute("from", "");
    QString to = xmlElement.attribute("to");

    m_streamNegotiationVariableMap->value(streamId)->setHost(to);
    //emit sigHost(m_host);

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
    if (!m_streamNegotiationVariableMap->value(streamId)->firstFeatureProceed())
    {
        reply += firstFeatures();
    }
    else if (!m_streamNegotiationVariableMap->value(streamId)->secondChallengeGenerated())
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

    QByteArray reply = document.toByteArray() + QByteArray(",") +
            m_serverConfigMap->value("ssl").toMap().value("certificate").toByteArray() + QByteArray(",") +
            m_serverConfigMap->value("ssl").toMap().value("key").toByteArray();
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
QByteArray StreamNegotiationManager::generateFirstChallengeReply(QByteArray clientXml, QString streamId)
{
    QDomDocument document;
    document.setContent(clientXml);

    QDomElement auth = document.documentElement();

    QString mechanism = auth.attribute("mechanism");

    QByteArray firstChallenge;
    if (mechanism == "DIGEST-MD5")
    {
        firstChallenge += "realm=";
        firstChallenge += m_streamNegotiationVariableMap->value(streamId)->getHost();
        firstChallenge += ",nonce=";
        firstChallenge += Sasl::generateNonce();
        firstChallenge += ",qop=auth";
        firstChallenge += ",charset=utf-8,algorithm=md5-sess";
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
QByteArray StreamNegotiationManager::generateSecondChallengeReply(QString streamId)
{
    QByteArray secondChallenge = "rspauth=" + Sasl::generateResponseValue(m_streamNegotiationVariableMap->value(streamId)->getClientFirstResponseDataMap());

    QDomDocument replyDocument;

    QDomElement challenge = replyDocument.createElement("challenge");
    challenge.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
    challenge.appendChild(replyDocument.createTextNode(secondChallenge.toBase64()));

    replyDocument.appendChild(challenge);

    return replyDocument.toByteArray();
}

bool StreamNegotiationManager::parse(QByteArray clientResponse, QString streamId)
{
    QMultiHash<QString, QByteArray> clientResponseDataMap;
    QList<QByteArray> m_clientFirstResponseMapList = clientResponse.split(',');
    foreach (QByteArray data, m_clientFirstResponseMapList)
    {
        QString key = QString(data.left(data.indexOf('=')));
        QByteArray value = data.mid(data.indexOf('=') + 1).replace(QByteArray("\""), QByteArray(""));
        clientResponseDataMap.insert(key, value);
    }

    if ((clientResponseDataMap.count("nonce") > 1) || (clientResponseDataMap.count("cnonce") > 1)
            || (!clientResponseDataMap.value("authzid").isEmpty())
            || (clientResponseDataMap.value("nonce-count") == "0000001"))
    {
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
