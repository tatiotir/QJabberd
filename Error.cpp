#include "Error.h"

Error::Error()
{
}

/*!
 * \brief The Error::generateError method generate stanza error function to the stanza type according to the XMPP core
 * \param stanzaType
 * \param errorType
 * \param errorCause
 * \param from
 * \param to
 * \param id
 * \param request
 * \return QByteArray
 */
QByteArray Error::generateError(QString by, QString stanzaType, QString errorType, QString errorCause,
                                QString from, QString to, QString id, QDomElement request)
{
    QDomDocument document;
    QDomElement element = document.createElement(stanzaType);

    if (!from.isEmpty())
    {
        element.setAttribute("from", from);
    }
    if (!to.isEmpty())
    {
        element.setAttribute("to", to);
    }
    if (!request.isNull())
    {
        element.appendChild(document.importNode(request, true));
    }

    element.setAttribute("type", "error");
    element.setAttribute("id", id);

    QDomElement errorElement = document.createElement("error");
    errorElement.setAttribute("type", errorType);

    if (!by.isEmpty())
        errorElement.setAttribute("by", by);

    QDomElement errorCauseElement = document.createElement(errorCause);
    errorCauseElement.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");

    errorElement.appendChild(errorCauseElement);
    element.appendChild(errorElement);
    document.appendChild(element);

    return document.toByteArray();
}

QByteArray Error::generateError(QString by, QString stanzaType, QString errorType, QString errorCause,
                                QString specificErrorCause, QString specificErrorXmlns, QString feature,
                                QString from, QString to, QString id)
{
    QDomDocument document;
    QDomElement element = document.createElement(stanzaType);

    if (!from.isEmpty())
    {
        element.setAttribute("from", from);
    }
    if (!to.isEmpty())
    {
        element.setAttribute("to", to);
    }

    element.setAttribute("type", "error");
    element.setAttribute("id", id);

    QDomElement errorElement = document.createElement("error");
    errorElement.setAttribute("type", errorType);

    if (!by.isEmpty())
        errorElement.setAttribute("by", by);

    QDomElement errorCauseElement = document.createElement(errorCause);
    errorCauseElement.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");

    QDomElement specificErrorCauseElement = document.createElement(specificErrorCause);
    specificErrorCauseElement.setAttribute("xmlns", specificErrorXmlns);

    if (!feature.isEmpty())
        specificErrorCauseElement.setAttribute("feature", feature);

    errorElement.appendChild(errorCauseElement);
    errorElement.appendChild(specificErrorCauseElement);
    element.appendChild(errorElement);
    document.appendChild(element);

    return document.toByteArray();
}

/*!
 * \brief The Error::generateSmError method generate the stream management error
 * \param childName
 * \return QByteArray
 */
QByteArray Error::generateSmError(QString childName)
{
    QDomDocument document;
    QDomElement failed = document.createElement("failed");
    failed.setAttribute("xmlns", "urn:xmpp:sm:3");

    QDomElement child = document.createElement(childName);
    child.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");

    failed.appendChild(child);
    document.appendChild(failed);

    return document.toByteArray();
}

/*!
 * \brief The Error::generateSaslError method generate sasl error according to the XMPP Core
 * \param childName
 * \return QByteArray
 */
QByteArray Error::generateFailureError(QString xmlns, QString errorCause)
{
    QDomDocument document;
    QDomElement failure = document.createElement("failure");
    failure.setAttribute("xmlns", xmlns);

    failure.appendChild(document.createElement(errorCause));
    document.appendChild(failure);

    return document.toByteArray();
}

/*!
 * \brief The Error::generateStreamError method generate stream error during stream negotiation according to the XMPP Core
 * \param errorCause
 * \return QByteArray
 */
QByteArray Error::generateStreamError(QString errorCause)
{
    QDomDocument errorDocument;
    QDomElement errorElement = errorDocument.createElement("stream:error");

    QDomElement errorCauseElement = errorDocument.createElement(errorCause);
    errorCauseElement.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-streams");

    errorElement.appendChild(errorCauseElement);
    errorDocument.appendChild(errorElement);

    return errorDocument.toByteArray();
}

/*!
 * \brief The Error::generateInternalServerError generate internal server error according to the XMPP Core
 * \return QByteArray
 */
QByteArray Error::generateInternalServerError()
{
    QDomDocument document;
    QDomElement errorElement = document.createElement("error");
    errorElement.setAttribute("type", "wait");
    errorElement.setAttribute("code", 500);

    QDomElement errorCause = document.createElement("internal-server-error");
    errorCause.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");

    errorElement.appendChild(errorCause);
    document.appendChild(errorElement);
    return document.toByteArray();
}
