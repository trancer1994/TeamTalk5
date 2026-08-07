#include "AACToggle.h"

#include <QPainter>
#include <QPaintEvent>

AACToggle::AACToggle(const QString& label,
                     AACAccessibilityManager* aac,
                     QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
    , m_label(label)
{
    setMinimumHeight(48);
    setFocusPolicy(Qt::NoFocus);

    // AAC input controller
    m_input = m_aac->inputController();

    // AAC activation (scanning, dwell, deep‑well)
    connect(m_input, &AACInputController::activateItem,
            this, &AACToggle::activate);

    // AAC highlight (scanning focus ring)
    connect(m_input, &AACInputController::focusChanged,
            this, &AACToggle::updateHighlight);

    // AAC help mode
    connect(m_input, &AACInputController::helpRequested,
            this, &AACToggle::speakHelp);

    // Accessibility name
    setAccessibleName(label);

    // High‑contrast QSS hook
    setProperty("aacToggle", true);
}

void AACToggle::setChecked(bool on)
{
    if (m_checked == on)
        return;

    m_checked = on;
    update();
    emit toggled(on);

    // --- AAC feedback ---
    if (m_aac->feedbackEngine())
        m_aac->feedbackEngine()->playToggle(on, m_aac->modes().fatigueMode);

    // --- AAC speech ---
    if (m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        m_aac->speechEngine()->speakNotification(
            on ? tr("Enabled") : tr("Disabled"),
            SpeechPriority::System
        );
    }
}

void AACToggle::activate()
{
    // Only activate if this item is focused by AACInputController
    if (!m_input->isFocused(this))
        return;

    // Fatigue‑mode accidental activation guard
    if (m_aac->modes().fatigueMode) {
        if (!m_input->fatigueGuardPassed())
            return;
    }

    // Deep‑well semantics (long‑press)
    if (m_input->isDeepWell())
        speakHelp();

    // Toggle state
    setChecked(!m_checked);
}

void AACToggle::updateHighlight()
{
    bool nowHighlighted = m_input->isFocused(this);
    if (m_highlighted != nowHighlighted) {
        m_highlighted = nowHighlighted;
        update();
    }
}

void AACToggle::speakHelp()
{
    if (!m_aac->modes().helpMode)
        return;

    if (m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        m_aac->speechEngine()->speak(
            tr("%1 toggle. Currently %2.")
                .arg(m_label)
                .arg(m_checked ? tr("enabled") : tr("disabled"))
        );
    }
}

void AACToggle::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRect r = rect().adjusted(4, 4, -4, -4);

    // Background
    p.fillRect(rect(), QColor("#f0f0f0"));

    // AAC highlight ring
    if (m_highlighted) {
        p.setPen(QPen(QColor("#0078d7"), 3));
        p.drawRoundedRect(r, 6, 6);
    }

    // Checked indicator
    if (m_checked) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#0078d7"));
        p.drawEllipse(width() - 32, height() / 2 - 8, 16, 16);
    }

    // Label
    p.setPen(Qt::black);
    p.drawText(rect().adjusted(12, 0, -12, 0),
               Qt::AlignVCenter | Qt::AlignLeft,
               m_label);
}
