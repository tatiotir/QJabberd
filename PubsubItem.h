#ifndef PUBSUBITEM_H
#define PUBSUBITEM_H

#include <QString>
#include <QJsonObject>

class PubsubItem
{
public:
    PubsubItem(QString id, QByteArray data);
    PubsubItem();

    QString id() const;
    void setId(const QString &id);

    QByteArray data() const;
    void setData(const QByteArray &data);

    QJsonObject toJsonObject();
    static PubsubItem fromJsonObject(QJsonObject object);

private:
    QString m_id;
    QByteArray m_data;
};

#endif // PUBSUBITEM_H
