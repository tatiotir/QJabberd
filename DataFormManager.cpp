#include "DataFormManager.h"

DataFormManager::DataFormManager()
{
}

QDomElement DataFormManager::generateDataForm(QString formType, QString title, QString instruction,
                                               QList<DataFormField> dataFormFields)
{
    QDomDocument document;

    QDomElement xElement = document.createElement("x");
    xElement.setAttribute("xmlns", "jabber:x:data");
    xElement.setAttribute("type", formType);

    QDomElement titleElement = document.createElement("title");
    titleElement.appendChild(document.createTextNode(title));

    QDomElement instructionElement = document.createElement("instructions");
    instructionElement.appendChild(document.createTextNode(instruction));

    xElement.appendChild(titleElement);
    xElement.appendChild(instructionElement);
    foreach (DataFormField field, dataFormFields)
    {
        xElement.appendChild(field.toXmlElement());
    }
    return xElement;
}

QMap<QString, QVariant> DataFormManager::parseDataForm(QDomElement xElement)
{
    QMap<QString, QVariant> fieldMap;
    QDomNodeList fieldNodeList = xElement.elementsByTagName("field");
    for (int i = 0; i < fieldNodeList.count(); ++i)
    {
        QString var = fieldNodeList.item(i).toElement().attribute("var");
        if ((var == "muc#roomconfig_getmemberlist") || (var == "muc#roomconfig_roomadmins") ||
                (var == "muc#roomconfig_roomowners") || (var == "muc#roomconfig_presencebroadcast"))
        {

            QDomNodeList valueNodeList = fieldNodeList.item(i).toElement().elementsByTagName("value");

            QList<QString> valueList;
            for (int i = 0; i < valueNodeList.count(); ++i)
            {
                valueList << valueNodeList.item(i).toElement().text();
            }
            fieldMap.insert(var, QVariant(valueList));
        }
        else
        {
            fieldMap.insert(var, fieldNodeList.item(i).toElement().elementsByTagName("value").item(0).toElement().text());
        }
    }
    return fieldMap;
}

QDomDocument DataFormManager::getRegistrationForm(QString id)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", "result");
    iqNode.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:register");

    // Generate data form
    QString formType = "form";
    QString formTitle = "Creating a new QJabberd account";
    QString formInstruction = "Choose a username and password for use with this service.";

    QList<DataFormField> formFields;
    formFields << DataFormField("FORM_TYPE", "hidden", "", QVariant("jabber:iq:register"), QList<QPair<QString, QString> >(), false)
                  << DataFormField("username", "text-single", "Username", QVariant(""), QList<QPair<QString, QString> >(), true)
                     << DataFormField("password", "text-private", "Password", QVariant(""), QList<QPair<QString, QString> >(), true);

    QDomElement xElement = generateDataForm(formType, formTitle, formInstruction, formFields);

    QDomElement instructionNode = document.createElement("instructions");
    QString instruction = "Choose a username and password for use with this service.";
    instructionNode.appendChild(document.createTextNode(instruction));

    query.appendChild(instructionNode);
    query.appendChild(document.createElement("username"));
    query.appendChild(document.createElement("password"));
    query.appendChild(xElement);
    iqNode.appendChild(query);
    document.appendChild(iqNode);

    return document;
}

QDomDocument DataFormManager::getPasswordChangeForm(QString from, QString to, QString id)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", "error");
    iqNode.setAttribute("from", from);
    iqNode.setAttribute("to", to);
    iqNode.setAttribute("id", id);

    // Generate data form
    QString formType = "form";
    QString formTitle = "Change user password for QJabberd account";
    QString formInstruction = "Use this form to change your password.";

    QList<DataFormField> formFields;
    formFields << DataFormField("FORM_TYPE", "hidden", "", QVariant("jabber:iq:register:changepassword"), QList<QPair<QString, QString> >(), false)
                  << DataFormField("username", "text-single", "Username", QVariant(""), QList<QPair<QString, QString> >(), true)
                     << DataFormField("old_password", "text-private", "Old Password", QVariant(""), QList<QPair<QString, QString> >(), true)
                        << DataFormField("new_password", "text-private", "New Password", QVariant(""), QList<QPair<QString, QString> >(), true);

    QDomElement xElement = generateDataForm(formType, formTitle, formInstruction, formFields);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:register");

    QDomElement titleNode = document.createElement("title");
    titleNode.appendChild(document.createTextNode("Change user password for QJabberd account"));

    QDomElement instructionNode = document.createElement("instructions");
    QString instruction = "Use this form to change your password.";
    instructionNode.appendChild(document.createTextNode(instruction));

    QDomElement errorNode = document.createElement("error");
    errorNode.setAttribute("code", 401);
    errorNode.setAttribute("type", "modify");

    QDomElement errorChild = document.createElement("not-authorized");
    errorChild.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");
    errorNode.appendChild(errorChild);

    query.appendChild(titleNode);
    query.appendChild(instructionNode);
    query.appendChild(xElement);
    iqNode.appendChild(query);
    iqNode.appendChild(errorNode);
    document.appendChild(iqNode);

    return document;
}

QDomDocument DataFormManager::getRoomConfigForm(QString type, QString from, QString to, QString id,
                                                QMap<QString, QVariant> dataFormValue)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", "result");
    iqNode.setAttribute("from", from);
    iqNode.setAttribute("to", to);
    iqNode.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "http://jabber.org/protocol/muc#owner");

    // Generate data form
    QString formType = "form";
    QString formTitle = "Configuration for " + Utils::getUsername(from) + " Room";
    QString formInstruction = "Use this form to configure the room.";

    QList<DataFormField> formFields;
    formFields << DataFormField("FORM_TYPE", "hidden", "",
                                QVariant("http://jabber.org/protocol/muc#roomconfig"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_roomname", "text-single",
                                "Room Name", dataFormValue.value("muc#roomconfig_roomname"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_roomdesc", "text-single", "Room Description",
                                dataFormValue.value("muc#roomconfig_desc"), QList<QPair<QString, QString> >(),
                                false)
               << DataFormField("muc#roomconfig_lang", "text-single", "Language for the Room Discussion",
                                dataFormValue.value("muc#roomconfig_lang"), QList<QPair<QString, QString> >(),
                                false)
               << DataFormField("muc#maxhistoryfetch", "text-single",
                                "Maximum Number of History Message",
                                dataFormValue.value("muc#maxhistoryfetch"), QList<QPair<QString, QString> >(),
                                false)
               << DataFormField("muc#roomconfig_allowpm", "list-single", "Allow Send of Private Message",
                                dataFormValue.value("muc#roomconfig_allowpm"),
                                QList<QPair<QString, QString> >() << qMakePair(QString("Anyone"), QString("anyone")) << qMakePair(QString("Any one with voice"), QString("participants")) << qMakePair(QString("Moderators"), QString("moderators")) << qMakePair(QString("Nobody"), QString("none")),
                                false)
               << DataFormField("muc#roomconfig_allowinvites", "boolean", "Allow Occupants to Invite Others",
                                dataFormValue.value("muc#roomconfig_allowinvites"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_changesubject", "boolean", "Allow Occupants to Change Subject",
                                dataFormValue.value("muc#roomconfig_changesubject"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_getmemberlist", "list-multi",
                                "Roles and Affiliations that May Retrieve Member List",
                                dataFormValue.value("muc#roomconfig_getmemberlist"),
                                QList<QPair<QString, QString> >() << qMakePair(QString("Moderator"), QString("moderator")) << qMakePair(QString("Participant"), QString("participant")) << qMakePair(QString("Visitor"), QString("visitor")),
                                false)
               << DataFormField("muc#roomconfig_maxusers", "list-single", "Maximum Number of Room Occupants",
                                dataFormValue.value("muc#roomconfig_maxusers"),
                                QList<QPair<QString, QString> >() << qMakePair(QString("10"), QString("10")) << qMakePair(QString("20"), QString("20")) << qMakePair(QString("30"), QString("30")) << qMakePair(QString("50"), QString("50")) << qMakePair(QString("100"), QString("100")) << qMakePair(QString("None"), QString("0")),
                                false)
               << DataFormField("muc#roomconfig_membersonly", "boolean", "Make Room Members-Only",
                                dataFormValue.value("muc#roomconfig_membersonly"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_moderatedroom", "boolean", "Make Room Moderated",
                                dataFormValue.value("muc#roomconfig_moderatedroom"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_passwordprotectedroom", "boolean", "Password is Required to Enter",
                                dataFormValue.value("muc#roomconfig_passwordprotectedroom"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_persistentroom", "boolean", "Make Room Persistent",
                                dataFormValue.value("muc#roomconfig_persistentroom"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_publicroom", "boolean", "Allow Public Searching for Room",
                                dataFormValue.value("muc#roomconfig_publicroom"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_roomadmins", "jid-multi", "Full List of Room Admins",
                                dataFormValue.value("muc#roomconfig_roomadmins"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_roomowners", "jid-multi", "Full List of Room Owners",
                                dataFormValue.value("muc#roomconfig_roomowners"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_roomsecret", "text-private", "The Room Password",
                                dataFormValue.value("mmuc#roomconfig_roomsecret"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomconfig_presencebroadcast", "list-multi",
                                "Roles for which Presence is Broadcasted",
                                dataFormValue.value("muc#roomconfig_presencebroadcast"),
                                QList<QPair<QString, QString> >() << qMakePair(QString("Moderator"), QString("moderator")) << qMakePair(QString("Participant"), QString("participant")) << qMakePair(QString("Visitor"), QString("visitor")),
                                false);

    QDomElement xElement = generateDataForm(formType, formTitle, formInstruction, formFields);

    query.appendChild(xElement);
    iqNode.appendChild(query);
    document.appendChild(iqNode);

    return document;
}

QDomDocument DataFormManager::getRoomRegistrationForm(QString from, QString to, QString id, QString roomName)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", "result");
    iqNode.setAttribute("from", from);
    iqNode.setAttribute("to", to);
    iqNode.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:register");

    // Generate data form
    QString formType = "form";
    QString formTitle = roomName + " Registration";
    QString formInstruction = "Use this form to register wiht the room " + roomName;

    QList<DataFormField> formFields;
    formFields << DataFormField("FORM_TYPE", "hidden", "",
                                QVariant("http://jabber.org/protocol/muc#register"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#register_roomnick", "text-single", "Desired Nickname", QVariant(""),
                                QList<QPair<QString, QString> >(), false);

    QDomElement instructionNode = document.createElement("instructions");
    QString instruction = "Use this form to register wiht the room " + roomName;
    instructionNode.appendChild(document.createTextNode(instruction));

    QDomElement xElement = generateDataForm(formType, formTitle, formInstruction, formFields);

    query.appendChild(instructionNode);
    query.appendChild(xElement);
    document.appendChild(query);
    return document;
}

QDomDocument DataFormManager::getRoomVoiceRequestForm(QString from, QString to, QString id, QString role, QString jid, QString roomnick)
{
    QDomDocument document;

    QDomElement messageNode = document.createElement("message");
    messageNode.setAttribute("type", "result");
    messageNode.setAttribute("from", from);
    messageNode.setAttribute("to", to);
    messageNode.setAttribute("id", id);

    // Generate data form
    QString formType = "form";
    QString formTitle = "Voice Request";
    QString formInstruction = "To approve this request for voice, select the 'Grant voice to this person' checkbox";

    QList<DataFormField> formFields;
    formFields << DataFormField("FORM_TYPE", "hidden", "",
                                QVariant("http://jabber.org/protocol/muc#request"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#role", "text-single", "Requested role", QVariant(role),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#jid", "jid-single", "User jid", QVariant(jid),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#roomnick", "text-single", "Room nickname", QVariant(roomnick),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("muc#request_allow", "boolean", "Grant voice to this person", QVariant(false),
                                QList<QPair<QString, QString> >(), false);

    QDomElement xElement = generateDataForm(formType, formTitle, formInstruction, formFields);

    messageNode.appendChild(xElement);
    document.appendChild(messageNode);

    return document;
}
