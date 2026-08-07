#include "AACToggle.h"

AACToggle::AACToggle(AACAccessibilityManager* mgr, QWidget* parent)
    : AACKeyButton("", mgr, parent)
    , m_mgr(mgr)
{
    setCheckable(true);
    setProperty("aacToggle", true);
    applyVisualState();
}

void AACToggle::setChecked(bool on)
{
    if (m_checked == on)
        return;

    m_checked = on;
    AACKeyButton::setChecked(on);
    applyVisualState();
    emit toggled(on);
}

void AACToggle::setDangerLevel(DangerLevel level)
{
    m_danger = level;
    setProperty("aacDangerLevel", static_cast<int>(level));
    style()->unpolish(this);
    style()->polish(this);
}

void AACToggle::setRole(AACRoleHelper::AACRole role)
{
    m_role = role;
    setProperty("aacRole", static_cast<int>(role));
    style()->unpolish(this);
    style()->polish(this);
}

void AACToggle::setSpeechDescription(const QString& desc)
{
    m_speechDescription = desc;
}

void AACToggle::setHelpText(const QString& help)
{
    m_helpText = help;
    setToolTip(help);
}

void AACToggle::onActivated()
{
    bool targetState = !m_checked;
    requestToggle(targetState);
}

void AACToggle::onDeepWellTriggered()
{
    bool targetState = !m_checked;
    setChecked(targetState);
    playFeedbackOnConfirm(targetState);
    speakToggle(targetState);
    emit activated(targetState);
}

void AACToggle::requestToggle(bool targetState)
{
    const auto modes = m_mgr->modes();

    if (m_danger == DangerLevel::Normal) {
        setChecked(targetState);
        playFeedbackOnConfirm(targetState);
        speakToggle(targetState);
        emit activated(targetState);
        return;
    }

    if (m_danger == DangerLevel::VeryDangerous) {
        emit dialogRequested(targetState);
        return;
    }

    if (modes.scanning || modes.stepScanning) {
        return;
    }

    if (modes.dwellMode) {
        enableDeepWell(true);
        return;
    }

    if (deepWellEnabled()) {
        return;
    }

    setChecked(targetState);
    playFeedbackOnConfirm(targetState);
    speakToggle(targetState);
    emit activated(targetState);
}

void AACToggle::applyVisualState()
{
    setProperty("aacOn", m_checked);
    style()->unpolish(this);
    style()->polish(this);
}

void AACToggle::playFeedbackOnConfirm(bool newState)
{
    auto* fb = m_mgr->feedback();

    switch (m_role) {
    case AACRoleHelper::AACRole::SafetyToggle:
        fb->playConnectionLost();
        fb->hapticError();
        break;

    case AACRoleHelper::AACRole::TransportToggle:
        if (newState)
            fb->playTransmitOn();
        else
            fb->playTransmitOff();
        fb->hapticStrong();
        break;

    default:
        fb->playAdjust();
        fb->hapticSoft();
        break;
    }
}

void AACToggle::speakToggle(bool newState)
{
    auto* speech = m_mgr->speechEngine();
    if (!speech)
        return;

    QString phrase = m_speechDescription.isEmpty()
                     ? text()
                     : m_speechDescription;

    phrase += newState ? tr(" on") : tr(" off");
    speech->speakScanningItem(phrase);
}
