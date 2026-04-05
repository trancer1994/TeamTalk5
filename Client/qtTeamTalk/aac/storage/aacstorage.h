#pragma once

#include <QString>
#include <QJsonObject>

class AACAccessibilityManager;

class AACStorage
{
public:
    AACStorage();

    void hydrate(AACAccessibilityManager& mgr);
    void persist(const AACAccessibilityManager& mgr);

private:
    QString storagePath() const;

    QJsonObject loadJson() const;
    void saveJson(const QJsonObject& obj) const;
};
