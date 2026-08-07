#include "AACSettingsPanelHeader.h"
#include "AACKeyButton.h"
#include "AACAccessibilityManager.h"
#include "AACSpeechEngine.h"
#include "AACFeedbackEngine.h"

#include <QHBoxLayout>
#include <QLabel>

AACSettingsPanelHeader::AACSettingsPanelHeader(AACAccessibilityManager* mgr,
                                               const QString& title,
                                               const QString& helpText,
                                               QWidget* parent)
    : QWidget(parent)
    , m_mgr(mgr)
    , m_title(title)
    , m_helpText(helpText)
{
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(16, 16, 16, 16);
    row->setSpacing(24);

    // Back button (AACKeyButton)
    m_btnBack = new AACKeyButton(tr("Back"), mgr, this);
    m_btnBack->setProperty("aacBackButton", true);

    connect(m_btnBack, &AACKeyButton::keyActivated,
            this, &AACSettingsPanelHeader::backRequested);

    // Title label
    auto* lbl = new QLabel(title, this);
    lbl->setProperty("aacSettingsHeaderTitle", true);
    lbl->setWordWrap(true);

    row->addWidget(m_btnBack);
    row->addWidget(lbl, 1);

    applyVisuals();
    speakEntry();
}

void AACSettingsPanelHeader::applyVisuals()
{
    setProperty("aacSettingsHeader", true);
    style()->unpolish(this);
    style()->polish(this);
}

void AACSettingsPanelHeader::speakEntry()
{
    if (!m_mgr)
        return;

    auto* speech = m_mgr->speechEngine();
    auto* fb     = m_mgr->feedback();

    fb->playModeChange();
    fb->hapticSoft();

    QString phrase = m_title;
    if (!m_helpText.isEmpty())
        phrase += ". " + m_helpText;

    speech->speakNotification(phrase);
}
