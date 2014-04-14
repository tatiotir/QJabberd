/*
 * Author : Tatioti Mbogning Raoul
 * Description : This class implement TLS - SASL stream negotiation mechanism.
 */


#include "streamnegotiation.h"

StreamNegotiation::StreamNegotiation()
{
    m_tlsFeatureProceed = false;
    m_saslFeatureProceed = false;
}

QByteArray StreamNegotiation::reply(QByteArray clientXML)
{
    QDomDocument document;
    document.setContent(clientXML);

    if (document.documentElement().tagName() == "stream:stream")
    {
        return generateFirstStreamReply(clientXML);
    }
    else if (document.documentElement().tagName() == "starttls")
    {
        return generateStartTlsReply();
    }
    else if (document.documentElement().tagName() == "auth")
    {
        return generateFirstChallengeReply(clientXML);
    }
    else if (document.documentElement().tagName() == "response")
    {
        QString clientResponseValue = document.documentElement().text();

        if (!clientResponseValue.isEmpty())
        {
            // If client sended a good response
            if (this->parse(QByteArray::fromBase64(clientResponseValue.toUtf8())))
            {
                return generateSecondChallengeReply();
            }
            else
            {
                // Close the stream and stream negociation
                return QByteArray("");
            }
        }
        else
        {
            return generateSaslSuccessReply();
        }
    }
}

QByteArray StreamNegotiation::tlsFeature()
{
    QDomDocument document;

    QDomElement streamFeatures = document.createElement("stream:features");

    QDomElement starttls = document.createElement("starttls");
    starttls.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");

    QDomElement mechanisms = document.createElement("mechanisms");
    mechanisms.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

    QDomElement firstMechanism = document.createElement("mechanism");
    firstMechanism.appendChild(document.createTextNode("DIGEST-MD5"));

    QDomElement secondMechanism = document.createElement("mechanism");
    secondMechanism.appendChild(document.createTextNode("PLAIN"));

    mechanisms.appendChild(firstMechanism);
    mechanisms.appendChild(secondMechanism);

    starttls.appendChild(document.createElement("required"));

    streamFeatures.appendChild(starttls);
    streamFeatures.appendChild(mechanisms);

    document.appendChild(streamFeatures);

    return document.toByteArray();
}

QByteArray StreamNegotiation::saslFeature()
{
    QDomDocument document;

    QDomElement streamFeatures = document.createElement("stream:features");

    QDomElement mechanisms = document.createElement("mechanisms");
    mechanisms.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

    QDomElement firstMechanism = document.createElement("mechanism");
    firstMechanism.appendChild(document.createTextNode("DIGEST-MD5"));

    QDomElement secondMechanism = document.createElement("mechanism");
    secondMechanism.appendChild(document.createTextNode("PLAIN"));

    QDomElement thirdMechanism = document.createElement("mechanism");
    thirdMechanism.appendChild(document.createTextNode("EXTERNAL"));

    mechanisms.appendChild(firstMechanism);
    mechanisms.appendChild(secondMechanism);
    mechanisms.appendChild(thirdMechanism);

    streamFeatures.appendChild(mechanisms);

    document.appendChild(streamFeatures);

    return document.toByteArray();
}

QByteArray StreamNegotiation::bindFeature()
{
    QDomDocument document;

    QDomElement streamFeatures = document.createElement("stream:features");

    QDomElement bind = document.createElement("bind");
    bind.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-bind");

    streamFeatures.appendChild(bind);
    document.appendChild(streamFeatures);

    return document.toByteArray();
}

bool StreamNegotiation::tlsFeatureProceed()
{
    return m_tlsFeatureProceed;
}

bool StreamNegotiation::saslFeatureProceed()
{
    return m_saslFeatureProceed;
}

bool StreamNegotiation::bindFeatureProceed()
{
    return m_bindFeatureProceed;
}


QByteArray StreamNegotiation::generateFirstStreamReply(QByteArray clientXml)
{
    QDomDocument document;
    document.setContent(clientXml);

    QDomElement xmlElement = document.documentElement();

    QString from = xmlElement.attribute("from", "");
    QString to = xmlElement.attribute("to");

    m_host = to;

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

    return replyDocument.toByteArray().replace(QByteArray("/>"), QByteArray(">"));
}

QByteArray StreamNegotiation::generateStartTlsReply()
{
    QDomDocument document;

    QDomElement proceed = document.createElement("proceed");
    proceed.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");

    document.appendChild(proceed);

    return document.toByteArray();
}

QByteArray StreamNegotiation::generateSaslSuccessReply()
{
    QDomDocument replyDocument;

    QDomElement success = replyDocument.createElement("success");
    success.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

    replyDocument.appendChild(success);

    // We have done with sasl negiciation feature
    m_saslFeatureProceed = true;

    return replyDocument.toByteArray();
}

/*
 * This function generate first server challenge according to the digest mechanism for sasl authentification
 */
QByteArray StreamNegotiation::generateFirstChallengeReply(QByteArray clientXml)
{
    QDomDocument document;
    document.setContent(clientXml);

    QDomElement auth = document.documentElement();

    QString mechanism = auth.attribute("mechanism");

    QByteArray firstChallenge;
    if (mechanism == "DIGEST-MD5")
    {
        firstChallenge += "realm=";
        firstChallenge += m_host;
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
QByteArray StreamNegotiation::generateSecondChallengeReply()
{
    QByteArray secondChallenge = "rspauth=" + Sasl::generateResponseValue(m_map);

    QDomDocument replyDocument;

    QDomElement challenge = replyDocument.createElement("challenge");
    challenge.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
    challenge.appendChild(replyDocument.createTextNode(secondChallenge.toBase64()));

    replyDocument.appendChild(challenge);

    return replyDocument.toByteArray();
}

bool StreamNegotiation::parse(QByteArray decodedResponse)
{
    QList<QByteArray> m_clientFirstResponseMapList = decodedResponse.split(',');

    foreach (QByteArray data, m_clientFirstResponseMapList)
    {
        QString key = QString(data.left(data.indexOf('=')));
        QByteArray value = data.mid(data.indexOf('=') + 1).replace(QByteArray("\""), QByteArray(""));
        m_map.insert(key, value);
    }
    m_map.insert("password", QByteArray("20061992"));

    if ((m_map.count("nonce") > 1) || (m_map.count("cnonce") > 1) || (!m_map.value("authzid").isEmpty())
            || (m_map.value("nonce-count") == "0000001"))
    {
        return false;
    }
    return true;
}

void StreamNegotiation::tlsNegotiationSuccess()
{
    // Client have successful negociated tls feature
    m_tlsFeatureProceed = true;
}

QString StreamNegotiation::getUserJid()
{
    return m_map.value("username") + "@" + m_host;
}
