#include "Error.h"

Error::Error()
{
}

QByteArray Error::generateError(QString stanzaType, QString errorType, QString errorCause,
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
        element.appendChild(request);
    }

    element.setAttribute("type", "error");
    element.setAttribute("id", id);

    QDomElement errorElement = document.createElement("error");
    errorElement.setAttribute("type", errorType);

    QDomElement errorCauseElement = document.createElement(errorCause);
    errorCauseElement.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");

    errorElement.appendChild(errorCauseElement);
    element.appendChild(errorElement);
    document.appendChild(element);

    return document.toByteArray();
}

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

QByteArray Error::generateSaslError(QString childName)
{
    QDomDocument document;
    QDomElement failure = document.createElement("failure");
    failure.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");

    failure.appendChild(document.createElement(childName));
    document.appendChild(failure);

    return document.toByteArray();
}

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
