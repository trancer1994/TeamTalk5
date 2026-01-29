#pragma once

#include <QObject>
#include <QVector>
#include <QString>

namespace AAC {

struct SymbolEntry {
    QString label;
    QString iconPath;   // qrc:/ path
};

class AACGridModel : public QObject
{
    Q_OBJECT

public:
    explicit AACGridModel(QObject* parent = nullptr);

    void loadFromJson(const QJsonObject& json);

    const QVector<SymbolEntry>& entries() const { return m_entries; }

private:
    QVector<SymbolEntry> m_entries;
};

} // namespace AAC
