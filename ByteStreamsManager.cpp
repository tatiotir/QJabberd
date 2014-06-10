#include "ByteStreamsManager.h"

ByteStreamsManager::ByteStreamsManager(QJsonObject *serverConfiguration)
{
    m_serverConfiguration = serverConfiguration;
}

QByteArray ByteStreamsManager::byteStreamManagerReply(QDomDocument document, QString iqFrom)
{
    QDomElement iq = document.firstChildElement();
    QString from = iq.attribute("from", iqFrom);
    QString to = iq.attribute("to");
    QString id = iq.attribute("id", Utils::generateId());
    QString type = iq.attribute("type");

    if (type == "get")
    {
        QJsonObject proxyObject = getProxyObject(to);
        if (!proxyObject.isEmpty())
        {
            return byteStreamManagerResult(to, from, id, proxyObject);
        }
        else
        {
            return Error::generateError("", "iq", "cancel", "service-unavalaible",
                                        to, iqFrom, "", QDomElement());
        }
    }
    else if (type == "set")
    {
//        QString sid = iq.firstChildElement().attribute("sid");
//        QString mode = iq.firstChildElement().attribute("mode");
//        QString dstaddr = iq.firstChildElement().attribute("dstaddr");

//        if (sid.isEmpty())
//        {
//            return Error::generateError("", "iq", "modify", "not-acceptable", to, from, id, QDomElement());
//        }


    }
    return QByteArray();
}

QByteArray ByteStreamsManager::byteStreamManagerResult(QString from, QString to, QString id, QJsonObject proxyObject)
{
    QDomDocument document;
    QDomElement iqElement = document.createElement("iq");
    iqElement.setAttribute("from", from);
    iqElement.setAttribute("to", to);
    iqElement.setAttribute("id", id);
    iqElement.setAttribute("type", "result");

    QDomElement queryElement = document.createElement("query");
    queryElement.setAttribute("xmlns", "http://jabber.org/protocol/bytestreams");

    QDomElement streamHostElement = document.createElement("streamhost");
    streamHostElement.setAttribute("host", proxyObject.value("host").toString());
    streamHostElement.setAttribute("jid", proxyObject.value("jid").toString());
    streamHostElement.setAttribute("port", QString::number(proxyObject.value("port").toInt()));

    queryElement.appendChild(streamHostElement);
    iqElement.appendChild(queryElement);
    document.appendChild(iqElement);

    return document.toByteArray();

}

QJsonObject ByteStreamsManager::getProxyObject(QString proxyJid)
{
    foreach (QString host, m_serverConfiguration->value("services").toVariant().toMap().keys())
    {
        QJsonArray hostServiceList = m_serverConfiguration->value("services").toObject().value(host).toArray();
        for (int i = 0; i < hostServiceList.count(); ++i)
        {
            if (hostServiceList[i].toObject().value("jid") == proxyJid)
                return hostServiceList[i].toObject();
        }
    }
    return QJsonObject();
}
