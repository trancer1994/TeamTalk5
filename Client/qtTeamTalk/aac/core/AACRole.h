#pragma once
#include <QObject>

class AACRoleHelper : public QObject
{
    Q_OBJECT
public:
    enum class AACRole {
        SettingsToggle,
        TransportToggle,
        SafetyToggle
    };
    Q_ENUM(AACRole)
};
