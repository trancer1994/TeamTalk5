#include "AACSetupSummaryScreen.h"
#include "AACKeyButton.h"
#include <QVBoxLayout>

AACSetupSummaryScreen::AACSetupSummaryScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("Summary"));

    auto* layout = new QVBoxLayout(this);

auto* helpLabel = new QLabel(tr("Help & Guidance"), this);
layout->addWidget(helpLabel);

m_helpModeToggle = new AACToggle(tr("Speak help when entering screens"), this);
m_helpModeToggle->setMinimumHeight(48);
layout->addWidget(m_helpModeToggle);
registerInteractive(m_helpModeToggle);

// ⭐ Sub‑label under Help Mode toggle
auto* helpSublabel = new QLabel(
    tr("Automatically speaks guidance when entering screens."),
    this
);
helpSublabel->setWordWrap(true);
helpSublabel->setStyleSheet("color: #666; font-size: 13px;");
layout->addWidget(helpSublabel);

// ⭐ Learn more about Help Mode
auto* helpInfo = new AACKeyButton(aac, tr("Learn more about Help Mode"), this);
helpInfo->setMinimumHeight(40);
layout->addWidget(helpInfo);
registerInteractive(helpInfo);

connect(helpInfo, &AACKeyButton::activated, this, [this]() {
    if (m_accessibility->speechEngine())
        m_accessibility->speechEngine()->speak(
            tr("Help Mode automatically speaks guidance when entering screens. "
               "Press F12 at any time to hear contextual help.")
        );
});
    auto* finishBtn = new AACKeyButton(aac, tr("Finish"), this);
    layout->addWidget(finishBtn);

    connect(finishBtn, &AACKeyButton::activated, this, [this]() {
    m_cfg.helpMode = m_helpModeToggle->isChecked();
        emit setupComplete(m_cfg);
    });

    setLayout(layout);
}

void AACSetupSummaryScreen::setConfig(const AACProfileConfig& cfg)
{
    m_cfg = cfg;
}
