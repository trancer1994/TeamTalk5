#pragma once

#include <QObject>
#include <QHash>

#include "AACElementMetadata.h"

class QWidget;

class AACElementRegistry : public QObject
{
    Q_OBJECT
public:
    explicit AACElementRegistry(QObject* parent = nullptr)
        : QObject(parent)
    {}

    void setMetadata(QWidget* w, const AACElementMetadata& md)
    {
        m_map[w] = md;
        emit metadataChanged(w);
    }

    AACElementMetadata metadata(QWidget* w) const
    {
        return m_map.value(w);
    }

signals:
    void metadataChanged(QWidget* w);

private:
    QHash<QWidget*, AACElementMetadata> m_map;
};
