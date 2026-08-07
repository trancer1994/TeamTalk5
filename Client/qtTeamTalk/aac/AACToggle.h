#pragma once

#include "AACKeyButton.h"
#include "AACRole.h"
#include <QElapsedTimer>
#include <QTimer>

class AACAccessibilityManager;

class AACToggle : public AACKeyButton
{
    Q_OBJECT
public:
    enum class DangerLevel {
        Normal,
        Dangerous,
        VeryDangerous
    };
    Q_ENUM(DangerLevel)

    explicit AACToggle(AACAccessibilityManager* mgr,
                       QWidget* parent = nullptr);

    void setChecked(bool on);
    bool isChecked() const { return m_checked; }

    void setDangerLevel(DangerLevel level);
    DangerLevel dangerLevel() const { return m_danger; }

    void setRole(AACRoleHelper::AACRole role);
    AACRoleHelper::AACRole role() const { return m_role; }

    void setSpeechDescription(const QString& desc);
    QString speechDescription() const { return m_speechDescription; }

    void setHelpText(const QString& help);
    QString helpText() const { return m_helpText; }

signals:
    void toggled(bool on);
    void activated(bool on);
    void dialogRequested(bool target);

protected:
    void onActivated() override; // AACKeyButton activation path
    void onDeepWellTriggered() override; // AACKeyButton deep-well path

private:
    void requestToggle(bool targetState);
    void applyVisualState();
    void playFeedbackOnConfirm(bool newState);
    void speakToggle(bool newState);

    AACAccessibilityManager* m_mgr = nullptr;

    bool m_checked = false;
    DangerLevel m_danger = DangerLevel::Normal;
    AACRoleHelper::AACRole m_role = AACRoleHelper::AACRole::SettingsToggle;

    QString m_speechDescription;
    QString m_helpText;
};
