#include "aac_grid_model.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace AAC {

AACGridModel::AACGridModel(QObject* parent)
    : QObject(parent)
{
}

void AACGridModel::loadFromJson(const QJsonObject& json)
{
    m_entries.clear();

    QJsonArray items = json["items"].toArray();   // your JSON root key

    for (const QJsonValue& v : items) {
        QJsonObject obj = v.toObject();

        SymbolEntry entry;
        entry.label = obj["label"].toString();
        entry.iconPath = obj["icon"].toString();   // e.g. "qrc:/aac/Symbols/Core/eat.png"

        m_entries.append(entry);
    }

    qDebug() << "AACGridModel: Loaded" << m_entries.size() << "symbols";
}

} // namespace AAC
