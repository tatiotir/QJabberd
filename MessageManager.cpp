#include "MessageManager.h"

/*!
 * \brief The MessageManager::MessageManager constructor of the message manager
 * \param userManager
 * \param privacyListManager
 */
MessageManager::MessageManager(UserManager *userManager, PrivacyListManager *privacyListManager)
{
    m_userManager = userManager;
    m_privacyListManager = privacyListManager;
}

/*!
 * \brief The MessageManager::parseMessage method parse a message request from XMPP client
 * \param messageXML
 * \param messageFrom
 * \return QByteArray
 */
QByteArray MessageManager::parseMessage(QDomDocument document, QString messageFrom)
{
    QDomElement messageNode = document.firstChildElement();
    QString from = messageNode.attribute("from", messageFrom);
    QString to = messageNode.attribute("to");
    QString type = messageNode.attribute("type");
    QString xmlLang = messageNode.attribute("xml:lang");

    /*if (!m_userManager->userExists(Utils::getBareJid(to)))
    {
        return Error::generateServiceUnavailableError("iq", from, to, Utils::generateId());
    }*/

    if ((type == "chat") || (type == "normal"))
    {
        messageNode.setAttribute("from", from);
        emit sigNewChatMessage(to, document);
    }
    return QByteArray();
}
