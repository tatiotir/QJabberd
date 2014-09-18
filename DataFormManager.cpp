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
                (var == "muc#roomconfig_roomowners") || (var == "muc#roomconfig_presencebroadcast") ||
                (var == "pubsub#children_association_whitelist") || (var == "pubsub#children") ||
                (var == "pubsub#collection") || (var == "pubsub#contact") ||
                (var == "pubsub#roster_groups_allowed"))
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

QDomDocument DataFormManager::getNodeSubscriptionOptionForm(QString type, QString from, QString to, QString id, QString node,
                                                            QString jid, QMap<QString, QVariant> dataFormValue)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", type);
    iqNode.setAttribute("from", from);
    iqNode.setAttribute("to", to);
    iqNode.setAttribute("id", id);

    QDomElement pubsubElement = document.createElement("pubsub");
    pubsubElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub");

    QDomElement optionElement = document.createElement("option");
    optionElement.setAttribute("node", node);
    optionElement.setAttribute("jid", jid);

    // Generate data form
    QString formType = "form";
    QString formTitle = "Subscription configuration for " + node + " Node";
    QString formInstruction = "Use this form to configure your subscription to the node.";

    QList<DataFormField> formFields;
    formFields << DataFormField("FORM_TYPE", "hidden", "",
                                QVariant("http://jabber.org/protocol/pubsub#subscribe_options"),
                                QList<QPair<QString, QString> >(), false)
                  << DataFormField("pubsub#deliver", "boolean", "Enable delivery?", dataFormValue.value("pubsub#deliver"),
                                   QList<QPair<QString, QString> >(), false)
                     << DataFormField("pubsub#digest", "boolean", "Receive digest notifications (approx. one per day)?",
                                      dataFormValue.value("pubsub#digest"), QList<QPair<QString, QString> >(), false)
                        << DataFormField("pubsub#digest_frequency", "text-single", "The minimum number of milliseconds between sending any two notification digests",
                                         dataFormValue.value("pubsub#digest_frequency"), QList<QPair<QString, QString> >(), false)
                            << DataFormField("pubsub#include_body", "boolean", "Receive message body in addition to payload?",
                                         dataFormValue.value("pubsub#include_body"), QList<QPair<QString, QString> >(), false)
                                << DataFormField("pubsub#expire", "text-single", "The DateTime at which a leased subscription will end or has ended",
                                                 dataFormValue.value("pubsub#expire"), QList<QPair<QString, QString> >(), false)
                                   << DataFormField("pubsub#show-values", "list-multi", "The presence states for which an entity wants to receive notifications",
                                                    dataFormValue.value("pubsub#show-values"), QList<QPair<QString, QString> >() << qMakePair(QString("XMPP Show Value of Away"), QString("away")) <<
                                                    qMakePair(QString("XMPP Show Value of Chat"), QString("chat")) << qMakePair(QString("XMPP Show Value of DND (Do Not Disturb)"), QString("dnd")) <<
                                                    qMakePair(QString("Mere Availability in XMPP (No Show Value)"), QString("online")) << qMakePair(QString("XMPP Show Value of XA (Extended Away)"), QString("xa")), false)
                                        << DataFormField("pubsub#subscription_type", "list-single", "", dataFormValue.value("pubsub#subscription_type"), QList<QPair<QString, QString> >() << qMakePair(QString("Receive notification of new items only"), QString("items")) <<
                                                       qMakePair(QString("Receive notification of new nodes only"), QString("nodes")), false)
                                            << DataFormField("pubsub#subscription_depth", "list-single", "", dataFormValue.value("pubsub#subscription_depth"), QList<QPair<QString, QString> >() << qMakePair(QString("Receive notification from direct child nodes only"), QString("1")) <<
                                                             qMakePair(QString("Receive notification from all descendent nodes"), QString("all")), false);

    QDomElement xElement = generateDataForm(formType, formTitle, formInstruction, formFields);

    pubsubElement.appendChild(optionElement);
    optionElement.appendChild(xElement);
    iqNode.appendChild(pubsubElement);
    document.appendChild(iqNode);

    return document;
}

QDomDocument DataFormManager::getNodeConfigurationForm(QString type, QString from, QString to, QString id,
                                                       QString node, QMap<QString, QVariant> dataFormValue,
                                                       QSet<QString> userRosterGroups)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", type);
    iqNode.setAttribute("from", from);
    iqNode.setAttribute("to", to);
    iqNode.setAttribute("id", id);

    QDomElement pubsubElement = document.createElement("pubsub");
    pubsubElement.setAttribute("xmlns", "http://jabber.org/protocol/pubsub#owner");

    QDomElement configureElement = document.createElement("option");
    configureElement.setAttribute("node", node);

    QString formType = "form";
    QString formTitle = "Configuration for " + node + " Node";
    QString formInstruction = "Use this form to configure the node.";

    QList<DataFormField> formFields;
    formFields << DataFormField("FORM_TYPE", "hidden", "",
                                QVariant("http://jabber.org/protocol/pubsub#node_config"),
                                QList<QPair<QString, QString> >(), false)
                  << DataFormField("pubsub#title", "text-single", "A friendly name for the node", dataFormValue.value("pubsub#title"), QList<QPair<QString, QString> >(), false)
                     << DataFormField("pubsub#deliver_notifications", "boolean", "Whether to deliver event notifications", dataFormValue.value("pubsub#deliver_notifications"), QList<QPair<QString, QString> >(), false)
                        << DataFormField("pubsub#deliver_payloads", "boolean", "Whether to deliver payloads with event notifications", dataFormValue.value("pubsub#deliver_payloads"), QList<QPair<QString, QString> >(), false)
                           << DataFormField("pubsub#notify_config", "boolean", "Notify subscribers when the node configuration changes", dataFormValue.value("pubsub#notify_config"), QList<QPair<QString, QString> >(), false)
                              << DataFormField("pubsub#notify_delete", "boolean", "Notify subscribers when the node is deleted", dataFormValue.value("pubsub#notify_delete"), QList<QPair<QString, QString> >(), false)
                                 << DataFormField("pubsub#notify_retract", "boolean", "Notify subscribers when items are removed from the node", dataFormValue.value("pubsub#notify_retract"), QList<QPair<QString, QString> >(), false)
                                    << DataFormField("pubsub#notify_sub", "boolean", "Notify owners about new subscribers and unsubscribes", dataFormValue.value("pubsub#notify_sub"), QList<QPair<QString, QString> >(), false)
                                       << DataFormField("pubsub#persist_items", "boolean", "Persist items to storage", dataFormValue.value("pubsub#persist_items"), QList<QPair<QString, QString> >(), false)
                                          << DataFormField("pubsub#max_items", "text-single", "Max number of items to persist", dataFormValue.value("pubsub#max_items"), QList<QPair<QString, QString> >(), false)
                                             << DataFormField("pubsub#item_expire", "text-single", "Time after which to automatically purge items", dataFormValue.value("pubsub#item_expire"), QList<QPair<QString, QString> >(), false)
                                                << DataFormField("pubsub#subscribe", "boolean", "Whether to allow subscriptions", dataFormValue.value("pubsub#subscribe"), QList<QPair<QString, QString> >(), false)
                                                    << DataFormField("pubsub#access_model", "list-single", "Specify the subscriber model", dataFormValue.value("pubsub#access_model"), QList<QPair<QString, QString> >() << qMakePair(QString("Subscription requests must be approved and only subscribers may retrieve items"), QString("authorize")) <<
                                                        qMakePair(QString("Anyone may subscribe and retrieve items"), QString("open")) << qMakePair(QString("Anyone with a presence subscription of both or from may subscribe and retrieve items"), QString("presence")) <<
                                                        qMakePair(QString("Anyone in the specified roster group(s) may subscribe and retrieve items"), QString("roster")) << qMakePair(QString("Only those on a whitelist may subscribe and retrieve items"), QString("whitelist")), false);

    QList<QPair<QString, QString> > options;
    foreach (QString group, userRosterGroups)
    {
        options << qMakePair(QString(""), group);
    }

    formFields << DataFormField("pubsub#roster_groups_allowed", "list-multi", "Roster groups allowed to subscribe", dataFormValue.value("pubsub#roster_groups_allowed"), options, false)
                  << DataFormField("pubsub#publish_model", "list-single", "Specify the publisher model", dataFormValue.value("pubsub#publish_model"), QList<QPair<QString, QString> >() << qMakePair(QString(""), QString("publishers"))
                                   << qMakePair(QString(""), QString("subscribers")) << qMakePair(QString(""), QString("open")) << qMakePair(QString(""), QString("publishers")), false)
                     << DataFormField("pubsub#purge_offline", "boolean", "Purge all items when the relevant publisher goes offline?", dataFormValue.value("pubsub#purge_offline"), QList<QPair<QString, QString> >(), false)
                        << DataFormField("pubsub#max_payload_size", "text-single", "Max Payload size in bytes", dataFormValue.value("pubsub#max_payload_size"), QList<QPair<QString, QString> >(), false)
                           << DataFormField("pubsub#send_last_published_item", "list-single", "When to send the last published item", dataFormValue.value("pubsub#send_last_published_item"), QList<QPair<QString, QString> >() << qMakePair(QString("Never"), QString("never"))
                                            << qMakePair(QString("When a new subscription is processed"), QString("on_sub")) << qMakePair(QString("When a new subscription is processed and whenever a subscriber comes online"), QString("on_sub_and_presence")), false)
                              << DataFormField("pubsub#presence_based_delivery", "boolean", "Deliver event notifications only to available users", dataFormValue.value("pubsub#presence_based_delivery"), QList<QPair<QString, QString> >(), false)
                                 << DataFormField("pubsub#notification_type", "list-single", "Specify the delivery style for event notifications", dataFormValue.value("pubsub#notification_type"), QList<QPair<QString, QString> >() << qMakePair(QString(""), QString("normal"))
                                                  << qMakePair(QString(""), QString("headline")), false)
                                    << DataFormField("pubsub#type", "text-single", "Specify the type of payload data to be provided at this node", dataFormValue.value("pubsub#type"), QList<QPair<QString, QString> >(), false);

    QDomElement xElement = generateDataForm(formType, formTitle, formInstruction, formFields);

    pubsubElement.appendChild(configureElement);
    configureElement.appendChild(xElement);
    iqNode.appendChild(pubsubElement);
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
    QString formType = type;
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

QDomDocument DataFormManager::getNodeSubscriptionApprovalForm(QString from, QString to, QString id, QString node, QString subscriberJid)
{
    QDomDocument document;

    QDomElement messageNode = document.createElement("message");
    messageNode.setAttribute("from", from);
    messageNode.setAttribute("to", to);
    messageNode.setAttribute("id", id);

    // Generate data form
    QString formType = "form";
    QString formTitle = "PubSub subscriber request";
    QString formInstruction = "To approve this entity&apos;s subscription request, click the OK button. To deny the request, click the cancel button.";

    QList<DataFormField> formFields;
    formFields << DataFormField("FORM_TYPE", "hidden", "",
                                QVariant("http://jabber.org/protocol/pubsub#subscribe_authorization"),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("pubsub#node", "text-single", "Node ID", QVariant(node),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("pusub#subscriber_jid", "jid-single", "Subscriber Address", QVariant(subscriberJid),
                                QList<QPair<QString, QString> >(), false)
               << DataFormField("pubsub#allow", "boolean", "Allow this JID to subscribe to this pubsub node?", QVariant(false),
                                QList<QPair<QString, QString> >(), false);

    QDomElement xElement = generateDataForm(formType, formTitle, formInstruction, formFields);

    messageNode.appendChild(xElement);
    document.appendChild(messageNode);

    return document;
}
