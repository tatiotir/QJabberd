#include "Utils.h"

Utils::Utils()
{
}

QMap<QByteArray, QByteArray> Utils::parseHttpRequest(QByteArray postData)
{
    //qDebug() << "Post Data : " << postData;
    QList<QByteArray> lines = postData.split('\n');

    QMap<QByteArray, QByteArray> values;
    for (int i = 1; i < lines.count() - 1; ++i)
    {
        QList<QByteArray> data = lines.value(i).split(':');
        values.insert(data.value(0), data.value(1).trimmed());
    }

    values.insert("body", lines.value(lines.count() - 1));
    return values;
}

QByteArray Utils::generateHttpResponseHeader(int contentLength)
{
    QByteArray header = "HTTP/1.1 200 OK\r\n";
    header += "Content-Type: text/xml; charset=utf-8\r\n";
    header += "Content-Length: ";
    header += QByteArray::number(contentLength) + "\r\n\r\n";
    return header;
}

QList<QByteArray> Utils::parseRequest(QByteArray data)
{
//    QDomDocument document;
//    if ((data.indexOf("<stream:stream", 0) < 0) && !document.setContent(data))
//        return QList<QByteArray>();

    QList<QByteArray> result;
    int fromIq = 0;
    int fromPresence = 0;
    int fromMessage = 0;
    int fromStream = 0;
    int fromStartTls = 0;
    int fromResponse = 0;
    int fromAuth = 0;
    int fromR = 0;
    int fromA = 0;
    int fromResume = 0;
    int fromAbort = 0;
    int fromChallenge = 0;
    int fromSuccess = 0;

    int dataCount = data.count();
    QList<int> indexDebutIq;
    QList<int> indexDebutPresence;
    QList<int> indexDebutMessage;
    QList<int> indexDebutStream;
    QList<int> indexDebutStartTls;
    QList<int> indexDebutResponse;
    QList<int> indexDebutAuth;
    QList<int> indexDebutR;
    QList<int> indexDebutA;
    QList<int> indexDebutResume;
    QList<int> indexDebutAbort;
    QList<int> indexDebutChallenge;
    QList<int> indexDebutSuccess;

    while ((fromIq != dataCount) || (fromPresence != dataCount) || (fromMessage != dataCount)
           || (fromStream != dataCount) || (fromStartTls != dataCount) || (fromResponse != dataCount)
           || (fromAuth != dataCount) || (fromR != dataCount) || (fromA != dataCount)
           || (fromResume != dataCount) || (fromAbort != dataCount) || (fromChallenge != dataCount)
           || (fromSuccess != dataCount))
    {
        if (fromIq != dataCount)
        {
            int indexIq = data.indexOf("<iq", fromIq);
            if (indexIq >= 0)
            {
                indexDebutIq << indexIq;
                fromIq = ++indexIq;
            }
            else
            {
                fromIq = dataCount;
            }
        }

        if (fromSuccess != dataCount)
        {
            int indexSuccess = data.indexOf("<success", fromSuccess);
            if (indexSuccess >= 0)
            {
                indexDebutSuccess << indexSuccess;
                fromSuccess = ++indexSuccess;
            }
            else
            {
                fromSuccess = dataCount;
            }
        }

        if (fromChallenge != dataCount)
        {
            int indexChallenge = data.indexOf("<challenge", fromChallenge);
            if (indexChallenge >= 0)
            {
                indexDebutChallenge << indexChallenge;
                fromChallenge = ++indexChallenge;
            }
            else
            {
                fromChallenge = dataCount;
            }
        }

        if (fromPresence != dataCount)
        {
            int indexPresence = data.indexOf("<presence", fromPresence);
            if (indexPresence >= 0)
            {
                indexDebutPresence << indexPresence;
                fromPresence = ++indexPresence;
            }
            else
            {
                fromPresence = dataCount;
            }
        }

        if (fromMessage != dataCount)
        {
            int indexMessage = data.indexOf("<message", fromMessage);
            if (indexMessage >= 0)
            {
                indexDebutMessage << indexMessage;
                fromMessage = ++indexMessage;
            }
            else
            {
                fromMessage = dataCount;
            }
        }

        if (fromStream != dataCount)
        {
            int indexStream = data.indexOf("<stream:stream", fromStream);
            if (indexStream >= 0)
            {
                indexDebutStream << indexStream;
                fromStream = ++indexStream;
            }
            else
            {
                fromStream = dataCount;
            }
        }

        if (fromStartTls != dataCount)
        {
            int indexStartTls = data.indexOf("<starttls", fromStartTls);
            if (indexStartTls >= 0)
            {
                indexDebutStartTls << indexStartTls;
                fromStartTls = ++indexStartTls;
            }
            else
            {
                fromStartTls = dataCount;
            }
        }

        if (fromResponse != dataCount)
        {
            int indexResponse = data.indexOf("<response", fromResponse);
            if (indexResponse >= 0)
            {
                indexDebutResponse << indexResponse;
                fromResponse = ++indexResponse;
            }
            else
            {
                fromResponse = dataCount;
            }
        }

        if (fromAuth != dataCount)
        {
            int indexAuth = data.indexOf("<auth", fromAuth);
            if (indexAuth >= 0)
            {
                indexDebutAuth << indexAuth;
                fromAuth = ++indexAuth;
            }
            else
            {
                fromAuth = dataCount;
            }
        }

        if (fromR != dataCount)
        {
            int indexR = data.indexOf("<r ", fromR);
            if (indexR >= 0)
            {
                indexDebutR << indexR;
                fromR = ++indexR;
            }
            else
            {
                fromR = dataCount;
            }
        }

        if (fromA != dataCount)
        {
            int indexA = data.indexOf("<a ", fromA);
            if (indexA >= 0)
            {
                indexDebutA << indexA;
                fromA = ++indexA;
            }
            else
            {
                fromA = dataCount;
            }
        }

        if (fromResume != dataCount)
        {
            int indexResume = data.indexOf("<resume", fromResume);
            if (indexResume >= 0)
            {
                indexDebutResume << indexResume;
                fromResume = ++indexResume;
            }
            else
            {
                fromResume = dataCount;
            }
        }

        if (fromAbort != dataCount)
        {
            int indexAbort = data.indexOf("<abort", fromAbort);
            if (indexAbort >= 0)
            {
                indexDebutAbort << indexAbort;
                fromAbort = ++indexAbort;
            }
            else
            {
                fromAbort = dataCount;
            }
        }
    }

    if (!indexDebutIq.isEmpty())
    {
        QList<QByteArray> iqList;
        for (int i = 0; i < indexDebutIq.count(); ++i)
        {
            int indexFinIq = data.indexOf("</iq>", indexDebutIq.value(i));

            if (indexFinIq < 0)
            {
                indexFinIq = data.indexOf("/>", indexDebutIq.value(i)) + 2;
                QByteArray iq;
                for (int j = indexDebutIq.value(i); j < indexFinIq; ++j)
                {
                    iq += data.at(j);
                }
                iqList << iq;
                //iqList << data.left(indexFinIq - indexDebutIq.value(i));
            }
            else
            {
                indexFinIq += 5;
                QByteArray iq;
                for (int j = indexDebutIq.value(i); j < indexFinIq; ++j)
                {
                    iq += data.at(j);
                }
                iqList << iq;
                //iqList << data.left(indexFinIq - indexDebutIq.value(i));
            }
        }
        result << iqList;
    }

    if (!indexDebutChallenge.isEmpty())
    {
        QList<QByteArray> challengeList;
        for (int i = 0; i < indexDebutChallenge.count(); ++i)
        {
            int indexFinChallenge = data.indexOf("</challenge>", indexDebutChallenge.value(i));

            if (indexFinChallenge < 0)
            {
                indexFinChallenge = data.indexOf("/>", indexDebutChallenge.value(i)) + 2;
                QByteArray challenge;
                for (int j = indexDebutChallenge.value(i); j < indexFinChallenge; ++j)
                {
                    challenge += data.at(j);
                }
                challengeList << challenge;
                //iqList << data.left(indexFinIq - indexDebutIq.value(i));
            }
            else
            {
                indexFinChallenge += 12;
                QByteArray challenge;
                for (int j = indexDebutChallenge.value(i); j < indexFinChallenge; ++j)
                {
                    challenge += data.at(j);
                }
                challengeList << challenge;
                //iqList << data.left(indexFinIq - indexDebutIq.value(i));
            }
        }
        result << challengeList;
    }

    if (!indexDebutPresence.isEmpty())
    {
        QList<QByteArray> presenceList;
        for (int i = 0; i < indexDebutPresence.count(); ++i)
        {
            int indexFinPresence = data.indexOf("</presence>", indexDebutPresence.value(i));

            if (indexFinPresence < 0)
            {
                indexFinPresence = data.indexOf("/>", indexDebutPresence.value(i)) + 2;
                QByteArray presence;
                for (int j = indexDebutPresence.value(i); j < indexFinPresence; ++j)
                {
                    presence += data.at(j);
                }
                presenceList << presence;
                //presenceList << data.left(indexFinPresence - indexDebutPresence.value(i));
            }
            else
            {
                indexFinPresence += 11;
                QByteArray presence;
                for (int j = indexDebutPresence.value(i); j < indexFinPresence; ++j)
                {
                    presence += data.at(j);
                }
                presenceList << presence;
                //presenceList << data.left(indexFinPresence - indexDebutPresence.value(i));
            }
        }
        result << presenceList;
    }


    if (!indexDebutMessage.isEmpty())
    {
        QList<QByteArray> messageList;
        for (int i = 0; i < indexDebutMessage.count(); ++i)
        {
            int indexFinMessage = data.indexOf("</message>", indexDebutMessage.value(i));

            if (indexFinMessage < 0)
            {
                indexFinMessage = data.indexOf("/>", indexDebutMessage.value(i)) + 2;
                QByteArray message;
                for (int j = indexDebutMessage.value(i); j < indexFinMessage; ++j)
                {
                    message += data.at(j);
                }
                messageList << message;
                //messageList << data.left(indexFinMessage - indexDebutMessage.value(i));
            }
            else
            {
                indexFinMessage += 10;
                QByteArray message;
                for (int j = indexDebutMessage.value(i); j < indexFinMessage; ++j)
                {
                    message += data.at(j);
                }
                messageList << message;
                //messageList << data.left(indexFinMessage - indexDebutMessage.value(i));
            }
        }
        result << messageList;
    }

    if (!indexDebutStream.isEmpty())
    {
        QList<QByteArray> streamList;
        for (int i = 0; i < indexDebutStream.count(); ++i)
        {
            int indexFinStream = data.indexOf(">", indexDebutStream.value(i)) + 1;
            QByteArray stream;
            for (int j = indexDebutStream.value(i); j < indexFinStream; ++j)
            {
                stream += data.at(j);
            }
            streamList << stream;
        }
        result << streamList;
    }

    if (!indexDebutStartTls.isEmpty())
    {
        QList<QByteArray> startTlsList;
        for (int i = 0; i < indexDebutStartTls.count(); ++i)
        {
            int indexFinStartTls = data.indexOf("/>", indexDebutStartTls.value(i)) + 2;
            QByteArray startTls;
            for (int j = indexDebutStartTls.value(i); j < indexFinStartTls; ++j)
            {
                startTls += data.at(j);
            }
            startTlsList << startTls;
        }
        result << startTlsList;
    }

    if (!indexDebutSuccess.isEmpty())
    {
        QList<QByteArray> successList;
        for (int i = 0; i < indexDebutSuccess.count(); ++i)
        {
            int indexFinSuccess = data.indexOf("/>", indexDebutSuccess.value(i)) + 2;
            QByteArray success;
            for (int j = indexDebutSuccess.value(i); j < indexFinSuccess; ++j)
            {
                success += data.at(j);
            }
            successList << success;
        }
        result << successList;
    }

    if (!indexDebutResponse.isEmpty())
    {
        QList<QByteArray> ResponseList;
        for (int i = 0; i < indexDebutResponse.count(); ++i)
        {
            int indexFinResponse = data.indexOf("</response>", indexDebutResponse.value(i));

            if (indexFinResponse < 0)
            {
                indexFinResponse = data.indexOf("/>", indexDebutResponse.value(i)) + 2;
                QByteArray Response;
                for (int j = indexDebutResponse.value(i); j < indexFinResponse; ++j)
                {
                    Response += data.at(j);
                }
                ResponseList << Response;
                //ResponseList << data.left(indexFinResponse - indexDebutResponse.value(i));
            }
            else
            {
                indexFinResponse += 10;
                QByteArray Response;
                for (int j = indexDebutResponse.value(i); j < indexFinResponse; ++j)
                {
                    Response += data.at(j);
                }
                ResponseList << Response;
                //ResponseList << data.left(indexFinResponse - indexDebutResponse.value(i));
            }
        }
        result << ResponseList;
    }

    if (!indexDebutAuth.isEmpty())
    {
        QList<QByteArray> authList;
        for (int i = 0; i < indexDebutAuth.count(); ++i)
        {
            int indexFinAuth = data.indexOf("</auth>", indexDebutAuth.value(i));

            if (indexFinAuth < 0)
            {
                indexFinAuth = data.indexOf("/>", indexDebutAuth.value(i)) + 2;
                QByteArray auth;
                for (int j = indexDebutAuth.value(i); j < indexFinAuth; ++j)
                {
                    auth += data.at(j);
                }
                authList << auth;
                //authList << data.left(indexFinAuth - indexDebutAuth.value(i));
            }
            else
            {
                indexFinAuth += 7;
                QByteArray auth;
                for (int j = indexDebutAuth.value(i); j < indexFinAuth; ++j)
                {
                    auth += data.at(j);
                }
                authList << auth;
                //authList << data.left(indexFinAuth - indexDebutAuth.value(i));
            }
        }
        result << authList;
    }

    if (!indexDebutR.isEmpty())
    {
        QList<QByteArray> rList;
        for (int i = 0; i < indexDebutR.count(); ++i)
        {
            int indexFinR = data.indexOf("/>", indexDebutR.value(i)) + 2;
            QByteArray r;
            for (int j = indexDebutR.value(i); j < indexFinR; ++j)
            {
                r += data.at(j);
            }
            rList << r;
        }
        result << rList;
    }

    if (!indexDebutA.isEmpty())
    {
        QList<QByteArray> aList;
        for (int i = 0; i < indexDebutA.count(); ++i)
        {
            int indexFinA = data.indexOf("/>", indexDebutA.value(i)) + 2;
            QByteArray a;
            for (int j = indexDebutA.value(i); j < indexFinA; ++j)
            {
                a += data.at(j);
            }
            aList << a;
        }
        result << aList;
    }

    if (!indexDebutResume.isEmpty())
    {
        QList<QByteArray> resumeList;
        for (int i = 0; i < indexDebutResume.count(); ++i)
        {
            int indexFinResume = data.indexOf("/>", indexDebutResume.value(i)) + 2;
            QByteArray resume;
            for (int j = indexDebutResume.value(i); j < indexFinResume; ++j)
            {
                resume += data.at(j);
            }
            resumeList << resume;
        }
        result << resumeList;
    }

    if (!indexDebutAbort.isEmpty())
    {
        QList<QByteArray> abortList;
        for (int i = 0; i < indexDebutAbort.count(); ++i)
        {
            int indexFinAbort = data.indexOf("/>", indexDebutAbort.value(i)) + 2;
            QByteArray abort;
            for (int j = indexDebutAbort.value(i); j < indexFinAbort; ++j)
            {
                abort += data.at(j);
            }
            abortList << abort;
        }
        result << abortList;
    }

    return result;
}

QString Utils::generateResource()
{
    return QString(QCryptographicHash::hash(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8(),
                                    QCryptographicHash::Md5).toHex());
}

QString Utils::generateId()
{
    return generateResource();
}

QString Utils::getBareJid(QString fullJid)
{
    return fullJid.split("/").value(0);
}

QString Utils::getResource(QString fullJid)
{
    return fullJid.split("/").value(1);
}

QString Utils::getUsername(QString fullJid)
{
    return fullJid.split("@").value(0);
}

QString Utils::getHost(QString fullJid)
{
    if (fullJid.contains("/"))
    {
        return fullJid.split("/").value(0).split("@").value(1);
    }
    else
    {
        return fullJid.split("@").value(1);
    }
}

QDomDocument Utils::generateMucNotificationMessage(QString type, QString from, QString to, QString id, QList<int> statusCode)
{
    QDomDocument document;
    QDomElement messageElement = document.createElement("presence");
    messageElement.setAttribute("from", from);
    messageElement.setAttribute("to", to);
    messageElement.setAttribute("id", id);

    if (!type.isEmpty())
        messageElement.setAttribute("type", type);

    QDomElement xElement = document.createElement("x");
    xElement.setAttribute("xmlns", "http://jabber.org/protocol/muc#user");

    if (!statusCode.isEmpty())
    {
        foreach (int code, statusCode)
        {
            QDomElement statusElement = document.createElement("status");
            statusElement.setAttribute("code", QString::number(code));
            xElement.appendChild(statusElement);
        }
    }

    messageElement.appendChild(xElement);
    document.appendChild(messageElement);
    return document;
}

QDomDocument Utils::generatePresence(QString type, QString from, QString to, QString id, QString affiliation,
                                     QString role, QString jid, QString nick, QList<int> statusCodes,
                                     QString status, QString show)
{
    QDomDocument document;
    QDomElement presenceElement = document.createElement("presence");
    presenceElement.setAttribute("from", from);
    presenceElement.setAttribute("to", to);
    presenceElement.setAttribute("id", id);

    if (!status.isEmpty())
    {
        QDomElement statusNode = document.createElement("status");
        statusNode.appendChild(document.createTextNode(status));
        presenceElement.appendChild(statusNode);
    }

    if (!show.isEmpty())
    {
        QDomElement showNode = document.createElement("show");
        showNode.appendChild(document.createTextNode(show));
        presenceElement.appendChild(showNode);
    }

    if (!type.isEmpty())
        presenceElement.setAttribute("type", type);

    QDomElement xElement = document.createElement("x");
    xElement.setAttribute("xmlns", "http://jabber.org/protocol/muc#user");

    QDomElement itemElement = document.createElement("item");
    itemElement.setAttribute("affiliation", affiliation);
    itemElement.setAttribute("role", role);

    if (!jid.isEmpty())
        itemElement.setAttribute("jid", jid);

    if (!nick.isEmpty())
        itemElement.setAttribute("nick", nick);

    if (!statusCodes.isEmpty())
    {
        foreach (int code, statusCodes)
        {
            QDomElement statusElement = document.createElement("status");
            statusElement.setAttribute("code", QString::number(code));
            xElement.appendChild(statusElement);
        }
    }

    xElement.appendChild(itemElement);
    presenceElement.appendChild(xElement);
    document.appendChild(presenceElement);

    return document;
}

QDomDocument Utils::generatePresence(QString type, QString from, QString to, QString id, QString show,
                                   QString priority, QMultiHash<QString, QString> status)
{
    QDomDocument document;

    QDomElement presence = document.createElement("presence");

    if (!type.isEmpty())
        presence.setAttribute("type", type);

    if (!from.isEmpty())
        presence.setAttribute("from", from);

    if (!to.isEmpty())
        presence.setAttribute("to", to);

    if (!id.isEmpty())
        presence.setAttribute("id", id);

    if (!show.isEmpty())
    {
        QDomElement showNode = document.createElement("show");
        showNode.appendChild(document.createTextNode(show));
        presence.appendChild(showNode);
    }

    if (!priority.isEmpty())
    {
        QDomElement priorityNode = document.createElement("priority");
        priorityNode.appendChild(document.createTextNode(priority));
        presence.appendChild(priorityNode);
    }

    if (!status.isEmpty())
    {
        QList<QString> keys = status.keys();
        for (int i = 0, c = keys.count(); i < c; ++i)
        {
            QDomElement statusNode = document.createElement("status");
            statusNode.setAttribute("xml:lang", status.value(keys.value(i)));
            statusNode.appendChild(document.createTextNode(status.value(keys.value(i))));
            presence.appendChild(statusNode);
        }
    }

    document.appendChild(presence);
    return document;
}

QDomDocument Utils::generatePresence(QString type, QString from, QString to, QString id, QString show,
                                   QString priority, QString status)
{
    QDomDocument document;

    QDomElement presence = document.createElement("presence");

    if (!type.isEmpty())
        presence.setAttribute("type", type);

    if (!from.isEmpty())
        presence.setAttribute("from", from);

    if (!to.isEmpty())
        presence.setAttribute("to", to);

    if (!id.isEmpty())
        presence.setAttribute("id", id);

    if (!show.isEmpty())
    {
        QDomElement showNode = document.createElement("show");
        showNode.appendChild(document.createTextNode(show));
        presence.appendChild(showNode);
    }

    if (!priority.isEmpty())
    {
        QDomElement priorityNode = document.createElement("priority");
        priorityNode.appendChild(document.createTextNode(priority));
        presence.appendChild(priorityNode);
    }

    if (!status.isEmpty())
    {
        QDomElement statusNode = document.createElement("status");
        statusNode.appendChild(document.createTextNode(status));
        presence.appendChild(statusNode);
    }

    document.appendChild(presence);
    return document;
}

QDomDocument Utils::generateRosterPush(QString to, QString id, QString jid, QString name,
                                                   QString subscription, QString ask, bool approved,
                                                   QSet<QString> groupList)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");

    iq.setAttribute("type", QString("set"));
    iq.setAttribute("id", id);
    iq.setAttribute("to", to);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:roster");

    QDomElement item = document.createElement("item");
    item.setAttribute("jid", jid);

    if (!name.isEmpty())
    {
        item.setAttribute("name", name);
    }
    if (!ask.isEmpty())
    {
        item.setAttribute("ask", ask);
    }
    if (!subscription.isEmpty())
    {
        item.setAttribute("subscription", subscription);
    }
    if (approved)
    {
        item.setAttribute("approved", "true");
    }

    if (!groupList.isEmpty())
    {
        foreach (QString group, groupList)
        {
            QDomElement groupNode = document.createElement("group");
            groupNode.appendChild(document.createTextNode(group));
            item.appendChild(groupNode);
        }
    }
    query.appendChild(item);
    iq.appendChild(query);
    document.appendChild(iq);

    return document;
}

QDomDocument Utils::generateBlockPush(QString to, QString id, QList<QString> items)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("to", to);
    iq.setAttribute("type", "set");
    iq.setAttribute("id", id);

    QDomElement blockElement = document.createElement("block");
    blockElement.setAttribute("xmlns", "urn:xmpp:blocking");

    foreach (QString jid, items)
    {
        QDomElement item = document.createElement("item");
        item.setAttribute("jid", jid);
        blockElement.appendChild(item);
    }
    iq.appendChild(blockElement);
    document.appendChild(iq);

    return document;
}

QDomDocument Utils::generateUnblockPush(QString to, QString id, QList<QString> items)
{
    QDomDocument document;
    QDomElement iq = document.createElement("iq");
    iq.setAttribute("to", to);
    iq.setAttribute("type", "set");
    iq.setAttribute("id", id);

    QDomElement unBlockElement = document.createElement("unblock");
    unBlockElement.setAttribute("xmlns", "urn:xmpp:blocking");

    if (!items.isEmpty())
    {
        foreach (QString jid, items)
        {
            QDomElement item = document.createElement("item");
            item.setAttribute("jid", jid);
            unBlockElement.appendChild(item);
        }
    }
    iq.appendChild(unBlockElement);
    document.appendChild(iq);

    return document;
}

/*
 * This function compute the digest value according to the XEP 0078 for Non-SASL Authentification.
 */
QByteArray Utils::digestCalculator(QString id, QString password)
{
    QString data = id + password;
    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha1);
    return hash.toHex().toLower();
}

int Utils::affiliationIntValue(QString affiliation)
{
    if (affiliation == "none")
        return 0;
    if (affiliation == "outcast")
        return 1;
    if (affiliation == "member")
        return 2;
    if (affiliation == "admin")
        return 3;
    if (affiliation == "owner")
        return 4;
}

QDomDocument Utils::generateMucInvitationMessage(QString from, QString to, QString id, QString inviterJid,
                                                 QString roomPassword, QString reason)
{
    QDomDocument document;
    QDomElement messageElement = document.createElement("message");
    messageElement.setAttribute("from", from);
    messageElement.setAttribute("to", to);
    messageElement.setAttribute("id", id);

    QDomElement xElement = document.createElement("x");
    xElement.setAttribute("xmlns", "http://jabber.org/protocol/muc#user");

    QDomElement inviteElement = document.createElement("invite");
    inviteElement.setAttribute("from", inviterJid);

    if (!roomPassword.isEmpty())
    {
        QDomElement passwordElement = document.createElement("password");
        passwordElement.appendChild(document.createTextNode(roomPassword));
        xElement.appendChild(passwordElement);
    }

    xElement.appendChild(inviteElement);
    messageElement.appendChild(xElement);
    document.appendChild(messageElement);

    return document;
}

QString Utils::hmac(QByteArray key, QByteArray baseString)
{
    return QMessageAuthenticationCode::hash(baseString, key, QCryptographicHash::Sha1).toHex();
//    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
//    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
//        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
//    }

//    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
//    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "\"
//    // ascii characters 0x36 ("6") and 0x5c ("\") are selected because they have large
//    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)

//    for (int i = 0; i < key.length(); i++) {
//        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
//        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
//    }

//    // result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseString ) ).toBase64
//    QByteArray total = outerPadding;
//    QByteArray part = innerPadding;
//    part.append(baseString);
//    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
//    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
//    return hashed.toBase64();
}

QString Utils::XOR(QByteArray a, QByteArray b)
{
    QString xo;
    for (int i = 0; i < a.count(); ++i)
    {
        xo[i] = a[i] ^ b[i];
    }
    return xo;
}
