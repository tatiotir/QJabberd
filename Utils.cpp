#include "Utils.h"

Utils::Utils()
{
}

QList<QByteArray> Utils::parseRequest(QByteArray data)
{
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

    while ((fromIq != dataCount) || (fromPresence != dataCount) || (fromMessage != dataCount)
           || (fromStream != dataCount) || (fromStartTls != dataCount) || (fromResponse != dataCount)
           || (fromAuth != dataCount) || (fromR != dataCount) || (fromA != dataCount)
           || (fromResume != dataCount) || (fromAbort != dataCount))
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

QByteArray Utils::generatePresence(QString type, QString from, QString to, QString id, QString show,
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
    return document.toByteArray();
}

QByteArray Utils::generatePresence(QString type, QString from, QString to, QString id, QString show,
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
    return document.toByteArray();
}

QByteArray Utils::generateRosterPush(QString to, QString id, QString jid, QString name,
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

    return document.toByteArray();
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
