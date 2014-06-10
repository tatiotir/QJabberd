#ifndef DATAFORMFIELD_H
#define DATAFORMFIELD_H

#include <QString>
#include <QVariant>
#include <QPair>
#include <QDomDocument>
#include <QDomElement>

class DataFormField
{
public:
    DataFormField(QString var, QString type, QString label, QVariant value, QList<QPair<QString, QString> > options,
                  bool required);
    DataFormField();

    QString var() const;
    void setVar(const QString &var);

    QString type() const;
    void setType(const QString &type);

    QString label() const;
    void setLabel(const QString &label);

    QVariant value() const;
    void setValue(const QVariant &value);

    bool required() const;
    void setRequired(bool required);

    QDomElement toXmlElement();

private:
    QString m_var;
    QString m_type;
    QString m_label;
    QVariant m_value;
    QList<QPair<QString, QString> > m_options;
    bool m_required;
};

#endif // DATAFORMFIELD_H
