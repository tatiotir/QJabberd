#include "DataFormField.h"

DataFormField::
DataFormField(QString var, QString type, QString label, QVariant value, QList<QPair<QString, QString> > options,
              bool required)
{
    m_var = var;
    m_type = type;
    m_label = label;
    m_value = value;
    m_required = required;
    m_options = options;
}

DataFormField::DataFormField()
{
    m_var = "";
    m_type = "";
    m_value = "";
    m_required = false;
    m_options = QList<QPair<QString, QString> >();
}

QString DataFormField::var() const
{
    return m_var;
}

void DataFormField::setVar(const QString &var)
{
    m_var = var;
}
QString DataFormField::type() const
{
    return m_type;
}

void DataFormField::setType(const QString &type)
{
    m_type = type;
}
QString DataFormField::label() const
{
    return m_label;
}

void DataFormField::setLabel(const QString &label)
{
    m_label = label;
}

QVariant DataFormField::value() const
{
    return m_value;
}

void DataFormField::setValue(const QVariant &value)
{
    m_value = value;
}
bool DataFormField::required() const
{
    return m_required;
}

void DataFormField::setRequired(bool required)
{
    m_required = required;
}

QDomElement DataFormField::toXmlElement()
{
    QDomDocument document;
    QDomElement fieldElement = document.createElement("field");
    fieldElement.setAttribute("type", m_type);
    fieldElement.setAttribute("var", m_var);

    if (!m_label.isEmpty())
        fieldElement.setAttribute("label", m_label);

    if ((m_type == "hidden") || (m_type == "text-single") || (m_type == "boolean") || (m_type == "text-private")
            || (m_type == "jid-single"))
    {
        if (!m_value.toString().isEmpty())
        {
            QDomElement valueElement = document.createElement("value");
            valueElement.appendChild(document.createTextNode(m_value.toString()));
            fieldElement.appendChild(valueElement);
        }

        if (m_required)
            fieldElement.appendChild(document.createElement("required"));
    }
    else if (m_type == "list-single")
    {
        if (!m_value.toString().isEmpty())
        {
            QDomElement valueElement = document.createElement("value");
            valueElement.appendChild(document.createTextNode(m_value.toString()));
            fieldElement.appendChild(valueElement);
        }

        if (!m_options.isEmpty())
        {
            for (int i = 0; i < m_options.count(); ++i)
            {
                QDomElement optionElement = document.createElement("option");
                optionElement.setAttribute("label", m_options.value(i).first);

                QDomElement valueElement = document.createElement("value");
                valueElement.appendChild(document.createTextNode(m_options.value(i).second));

                optionElement.appendChild(valueElement);
                fieldElement.appendChild(optionElement);
            }
        }
    }
    else if (m_type == "list-multi")
    {
        if (!m_value.toStringList().isEmpty())
        {
            foreach (QString value, m_value.toStringList())
            {
                QDomElement valueElement = document.createElement("value");
                valueElement.appendChild(document.createTextNode(value));
                fieldElement.appendChild(valueElement);
            }
        }

        if (!m_options.isEmpty())
        {
            for (int i = 0; i < m_options.count(); ++i)
            {
                QDomElement optionElement = document.createElement("option");
                optionElement.setAttribute("label", m_options.value(i).first);

                QDomElement valueElement = document.createElement("value");
                valueElement.appendChild(document.createTextNode(m_options.value(i).second));

                optionElement.appendChild(valueElement);
                fieldElement.appendChild(optionElement);
            }
        }
    }
    else if (m_type == "jid-multi")
    {
        if (!m_value.toStringList().isEmpty())
        {
            foreach (QString value, m_value.toStringList())
            {
                QDomElement valueElement = document.createElement("value");
                valueElement.appendChild(document.createTextNode(value));
                fieldElement.appendChild(valueElement);
            }
        }
    }
    return fieldElement;
}
