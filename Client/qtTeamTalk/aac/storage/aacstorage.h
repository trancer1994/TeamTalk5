#pragma once

#include <QMap>
#include <QString>
#include <QJsonObject>

class AACAccessibilityManager;

class AACStorage
{
public:
    AACStorage();

    // Helpers for AACAccessibilityManager
    int loadUserVolume(const QString& userId) const;
    void saveUserVolume(const QString& userId, int volume);
    void hydrate(AACAccessibilityManager& mgr);
    void persist(const AACAccessibilityManager& mgr);

private:
    QMap<QString,int> m_userVolumes;
    QString storagePath() const;

    QJsonObject loadJson() const;
    void saveJson(const QJsonObject& obj) const;
};
