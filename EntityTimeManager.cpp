#include "EntityTimeManager.h"

EntityTimeManager::EntityTimeManager(QObject *parent) : QObject(parent)
{

}

/*!
 * \brief The EntityTimeManager::entityTimeManagerReply method reply to an entity time request from a user.
 * \param iqXML
 * \param iqFrom
 * \return QByteArray
 */
QByteArray EntityTimeManager::entityTimeManagerReply(QDomDocument document, QString iqFrom)
{
    QString to = document.documentElement().attribute("to");
    QString id = document.documentElement().attribute("id");

    // We determine if the host of the to attribute is one of the host managed by qjabberd

    if (Utils::getHost(to) == "localhost")
    {
        QDateTime local(QDateTime::currentDateTime());
        QDateTime utc(local.toTimeSpec(Qt::UTC));

        int hours = local.time().hour() - utc.time().hour();
        QString tzo = (hours >= 0 ? (QString::number(hours) + ":00") : (QString::number(-1*hours) + ":00"));

        return entityTimeMananerResult(to, iqFrom, id, tzo, utc.toString(Qt::ISODate));
    }

    // TODO else remotly route the request to the appropriate server
    return QByteArray();
}

/*!
 * \brief The EntityTimeManager::entityTimeMananerResult method generate the appropriate result according to the XEP entity time
 * \param from
 * \param to
 * \param id
 * \param tzo
 * \param utc
 * \return QByteArray
 */
QByteArray EntityTimeManager::entityTimeMananerResult(QString from, QString to, QString id,
                                                      QString tzo, QString utc)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("type", "result");
    iq.setAttribute("from", from);
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);

    QDomElement timeElement = document.createElement("time");
    timeElement.setAttribute("xmlns", "urn:xmpp:time");

    QDomElement tzoElement = document.createElement("tzo");
    tzoElement.appendChild(document.createTextNode(tzo));

    QDomElement utcElement = document.createElement("utc");
    utcElement.appendChild(document.createTextNode(utc));

    timeElement.appendChild(tzoElement);
    timeElement.appendChild(utcElement);
    iq.appendChild(timeElement);
    document.appendChild(iq);

    // Request Acknowledgment of receipt
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();

}
