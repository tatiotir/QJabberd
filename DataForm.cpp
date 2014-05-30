#include "DataForm.h"

DataForm::DataForm()
{
}

QByteArray DataForm::getRegistrationForm(QString from, QString to, QString id)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", "result");
    iqNode.setAttribute("from", from);
    iqNode.setAttribute("to", to);
    iqNode.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:register");

    QDomElement instructionNode = document.createElement("instructions");
    QString instruction = "Use the enclosed form to register . If your Jabber client does ";
    instruction += "not support Data Forms , visit http: // www . shakespeare . lit / contests .php";
    instructionNode.appendChild(document.createTextNode(instruction));

    QDomElement xNode = document.createElement("x");
    xNode.setAttribute("xmlns", "jabber:x:data");
    xNode.setAttribute("type", "form");

    QDomElement titleNode = document.createElement("title");
    titleNode.appendChild(document.createTextNode("Contest Registration"));

    QDomElement instructionNode1 = document.createElement("instructions");
    QString instruction1 = "Please provide the following information to sign up for our special contests !";
    instructionNode1.appendChild(document.createTextNode(instruction1));

    QDomElement field1 = document.createElement("field");
    field1.setAttribute("type", "hidden");
    field1.setAttribute("var", "FORM_TYPE");
    QDomElement value1 = document.createElement("value");
    value1.appendChild(document.createTextNode("jabber:iq:register"));
    field1.appendChild(value1);

    QDomElement field2 = document.createElement("field");
    field2.setAttribute("type", "text-single");
    field2.setAttribute("label", "Given_name");
    field2.setAttribute("var", "first");
    field2.appendChild(document.createElement("required"));

    QDomElement field3 = document.createElement("field");
    field3.setAttribute("type", "text-single");
    field3.setAttribute("label", "Family_name");
    field3.setAttribute("var", "last");
    field3.appendChild(document.createElement("required"));

    QDomElement field4 = document.createElement("field");
    field4.setAttribute("type", "text-single");
    field2.setAttribute("label", "Email_address");
    field4.setAttribute("var", "email");
    field4.appendChild(document.createElement("required"));

    QDomElement field5 = document.createElement("field");
    field5.setAttribute("type", "list-single");
    field5.setAttribute("label", "Gender");
    field5.setAttribute("var", "x-gender");
    field5.appendChild(document.createElement("required"));

    QDomElement option1 = document.createElement("option");
    option1.setAttribute("label", "Male");
    QDomElement val1 = document.createElement("value");
    val1.appendChild(document.createTextNode("M"));

    QDomElement option2 = document.createElement("option");
    option2.setAttribute("label", "Female");
    QDomElement val2 = document.createElement("value");
    val2.appendChild(document.createTextNode("F"));

    field5.appendChild(option1);
    field5.appendChild(option2);

    xNode.appendChild(titleNode);
    xNode.appendChild(instructionNode1);
    xNode.appendChild(field1);
    xNode.appendChild(field2);
    xNode.appendChild(field3);
    xNode.appendChild(field4);
    xNode.appendChild(field5);

    query.appendChild(instructionNode);
    query.appendChild(xNode);
    iqNode.appendChild(query);
    document.appendChild(iqNode);

    return document.toByteArray();
}

QByteArray DataForm::getPasswordChangeForm(QString from, QString to, QString id)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", "error");
    iqNode.setAttribute("from", from);
    iqNode.setAttribute("to", to);
    iqNode.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:register");

    QDomElement instructionNode = document.createElement("instructions");
    QString instruction = "Use this form to change your password.";
    instructionNode.appendChild(document.createTextNode(instruction));

    QDomElement xNode = document.createElement("x");
    xNode.setAttribute("xmlns", "jabber:x:data");
    xNode.setAttribute("type", "form");

    QDomElement field1 = document.createElement("field");
    field1.setAttribute("type", "hidden");
    field1.setAttribute("var", "FORM_TYPE");
    QDomElement value1 = document.createElement("value");
    value1.appendChild(document.createTextNode("jabber:iq:register:changepassword"));
    field1.appendChild(value1);

    QDomElement field2 = document.createElement("field");
    field2.setAttribute("type", "text-single");
    field2.setAttribute("label", "username");
    field2.setAttribute("var", "username");
    field2.appendChild(document.createElement("required"));

    QDomElement field3 = document.createElement("field");
    field3.setAttribute("type", "text-private");
    field3.setAttribute("label", "Old password");
    field3.setAttribute("var", "old_password");
    field3.appendChild(document.createElement("required"));

    QDomElement field4 = document.createElement("field");
    field4.setAttribute("type", "text-private");
    field2.setAttribute("label", "New Password");
    field4.setAttribute("var", "new_password");
    field4.appendChild(document.createElement("required"));

    QDomElement errorNode = document.createElement("error");
    errorNode.setAttribute("code", 406);
    errorNode.setAttribute("type", "modify");
    QDomElement errorChild = document.createElement("not-acceptable");
    errorChild.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp - stanzas");

    errorNode.appendChild(errorChild);

    xNode.appendChild(field1);
    xNode.appendChild(field2);
    xNode.appendChild(field3);
    xNode.appendChild(field4);

    query.appendChild(instructionNode);
    query.appendChild(xNode);
    iqNode.appendChild(query);
    iqNode.appendChild(errorNode);
    document.appendChild(iqNode);

    return document.toByteArray();
}

QMultiHash<QString, QString> DataForm::parseDataForm(QDomElement xElement)
{

}

QDomDocument DataForm::getRoomConfigForm(QString from, QString to, QString id)
{
    QDomDocument document;

    QDomElement iqNode = document.createElement("iq");
    iqNode.setAttribute("type", "result");
    iqNode.setAttribute("from", from);
    iqNode.setAttribute("to", to);
    iqNode.setAttribute("id", id);

    QDomElement query = document.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:register");

    QDomElement instructionNode = document.createElement("instructions");
    QString instruction = "Use the enclosed form to register . If your Jabber client does ";
    instruction += "not support Data Forms , visit http: // www . shakespeare . lit / contests .php";
    instructionNode.appendChild(document.createTextNode(instruction));

    QDomElement xNode = document.createElement("x");
    xNode.setAttribute("xmlns", "jabber:x:data");
    xNode.setAttribute("type", "form");
}
