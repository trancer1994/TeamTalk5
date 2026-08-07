#include "AACSettingsPanel.h"
#include "AACToggle.h"
#include "AACKeyButton.h"
#include "AACAccessibilityManager.h"
#include "AACSpeechEngine.h"
#include "AACFeedbackEngine.h"

#include <QVBoxLayout>
#include <QLabel>

AACSettingsPanel::AACSettingsPanel(AACAccessibilityManager* mgr,
                                   QWidget* parent)
    : QWidget(parent)
    , m_mgr(mgr)
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(24); // large targets
    root->setContentsMargins(32, 32, 32, 32);

    buildFooter();
}

void AACSettingsPanel::addSection(const QString& title)
{
    auto* label = new QLabel(title, this);
    label->setProperty("aacSectionHeader", true);
    label->setWordWrap(true);

    layout()->addWidget(label);
    m_sections.append(label);
}

void AACSettingsPanel::addToggle(const QString& key, AACToggle* toggle)
{
    m_toggles.insert(key, toggle);
    m_working.insert(key, toggle->isChecked());

    layout()->addWidget(toggle);

    connect(toggle, &AACToggle::toggled,
            this, [this, key](bool on) {
        m_working[key] = on;
    });
}

void AACSettingsPanel::loadFrom(const QMap<QString, bool>& current)
{
    m_working = current;

    for (auto it = m_toggles.begin(); it != m_toggles.end(); ++it) {
        const QString& key = it.key();
        if (current.contains(key))
            it.value()->setChecked(current.value(key));
    }
}

QMap<QString, bool> AACSettingsPanel::workingCopy() const
{
    return m_working;
}

void AACSettingsPanel::buildFooter()
{
    m_btnApply = new AACKeyButton(tr("Save"), m_mgr, this);
    m_btnCancel = new AACKeyButton(tr("Cancel"), m_mgr, this);

    m_btnApply->setProperty("aacPrimary", true);
    m_btnCancel->setProperty("aacSecondary", true);

    connect(m_btnApply, &AACKeyButton::keyActivated,
            this, &AACSettingsPanel::onApply);

    connect(m_btnCancel, &AACKeyButton::keyActivated,
            this, &AACSettingsPanel::onCancel);

    layout()->addWidget(m_btnApply);
    layout()->addWidget(m_btnCancel);
}

void AACSettingsPanel::onApply()
{
    auto* fb = m_mgr->feedback();
    auto* speech = m_mgr->speechEngine();

    fb->playAdjust();
    fb->hapticStrong();

    speech->speakNotification(tr("Settings saved"));

    emit applied(m_working);
}

void AACSettingsPanel::onCancel()
{
    auto* fb = m_mgr->feedback();
    auto* speech = m_mgr->speechEngine();

    fb->playAdjust();
    fb->hapticSoft();

    speech->speakNotification(tr("Changes cancelled"));

    emit cancelled();
}
