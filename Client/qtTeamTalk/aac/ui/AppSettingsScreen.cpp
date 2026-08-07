#include "AppSettingsScreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>

#include "AACKeyButton.h"

AppSettingsScreen::AppSettingsScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    // --- AAC-native title bar ---
    setScreenTitle(tr("App Settings."));

    // --- Root layout ---
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    layout->setContentsMargins(16, 16, 16, 16);

    // Helper for consistent row layout
    auto addRow = [&](const QString& label, QWidget* field) {
        auto* row = new QHBoxLayout();
        row->setSpacing(12);

        auto* lbl = new QLabel(label, this);
        lbl->setMinimumHeight(48);

        field->setMinimumHeight(48);

        row->addWidget(lbl);
        row->addWidget(field);

        layout->addLayout(row);
        registerInteractive(field);
    };

    // =====================================================
    //  PROFILE
    // =====================================================
    layout->addWidget(new QLabel(tr("Profile"), this));
    layout->addSpacing(20);

m_nickname = new QLineEdit(this);
m_nickname->setInputMethodHints(Qt::ImhNoPredictiveText);
addRow(tr("Nickname:"), m_nickname);

    layout->addWidget(new QLabel(tr("Interaction"), this));
    layout->addSpacing(8);

    // Auto reconnect toggle
    m_autoReconnect = new AACToggle(tr("Auto-reconnect"), this);
    m_autoReconnect->setMinimumHeight(48);
    layout->addWidget(m_autoReconnect);
    registerInteractive(m_autoReconnect);
m_helpMode = new AACToggle(tr("Speak help when entering screens"), this);
m_helpMode->setMinimumHeight(48);
layout->addWidget(m_helpMode);
registerInteractive(m_helpMode);
    layout->addSpacing(20);

// ⭐ Sub‑label under Help Mode toggle
auto* helpSublabel = new QLabel(
    tr("Automatically speaks guidance when entering screens."),
    this
);
helpSublabel->setWordWrap(true);
helpSublabel->setStyleSheet("color: #666; font-size: 13px;");
layout->addWidget(helpSublabel);

// ⭐ Learn more about Help Mode
auto* helpInfo = new AACButton(tr("Learn more about Help Mode"), m_accessibility, this);
helpInfo->setMinimumHeight(40);
layout->addWidget(helpInfo);
registerInteractive(helpInfo);

connect(helpInfo, &AACButton::activated, this, [this]() {
    if (m_accessibility->speechEngine())
        m_accessibility->speechEngine()->speak(
            tr("Help Mode automatically speaks guidance when entering screens. "
               "Press F12 at any time to hear contextual help.")
        );
});
    // =====================================================
    //  APPEARANCE SECTION
    // =====================================================
    layout->addWidget(new QLabel(tr("Appearance"), this));
    layout->addSpacing(8);

    // Theme
    m_theme = new QComboBox(this);
    m_theme->addItem(tr("System default"));
    m_theme->addItem(tr("Light"));
    m_theme->addItem(tr("Dark"));
    m_theme->setMinimumHeight(48);
    m_theme->setMaxVisibleItems(10);
m_theme->setStyleSheet(
    "QComboBox { combobox-popup: 0; }"
    "QComboBox QAbstractItemView { animation: none; }"
);
    addRow(tr("Theme:"), m_theme);

    layout->addSpacing(20);

    // =====================================================
    //  LANGUAGE SECTION
    // =====================================================
    layout->addWidget(new QLabel(tr("Language"), this));
    layout->addSpacing(8);

    m_language = new QComboBox(this);
    m_language->addItem(tr("System default"));
    m_language->setMinimumHeight(48);
    m_language->setMaxVisibleItems(10);
m_language->setStyleSheet(
    "QComboBox { combobox-popup: 0; }"
    "QComboBox QAbstractItemView { animation: none; }"
);
    addRow(tr("Language:"), m_language);

    layout->addSpacing(24);

    // =====================================================
    //  ACTION BUTTONS
    // =====================================================

    // Save button
    m_saveButton = new AACButton(tr("Save"), m_accessibility, this);
    m_saveButton->setMinimumHeight(56);
    layout->addWidget(m_saveButton);
    registerInteractive(m_saveButton, true);

    // Back button
    m_backButton = new AACButton(tr("Back"), m_accessibility, this);
    m_backButton->setMinimumHeight(56);
    layout->addWidget(m_backButton);
    registerInteractive(m_backButton);

    // Signals
    connect(m_backButton, &QPushButton::clicked, this, &AppSettingsScreen::backRequested);
    connect(m_saveButton, &QPushButton::clicked, this, &AppSettingsScreen::applySettings);
connect(m_helpMode, &AACToggle::toggled,
        this, &AppSettingsScreen::applySettings);

// Load saved profile values
const auto& profile = m_accessibility->userProfile();

m_nickname->setText(profile.nickname);

m_theme->setCurrentIndex(profile.theme);
m_language->setCurrentIndex(profile.language);

// Load auto‑reconnect
AACModeFlags modes = m_accessibility->modes();
m_autoReconnect->setChecked(modes.autoReconnect);
m_helpMode->setChecked(modes.helpMode);   // ⭐ ADD THIS
}
QString AppSettingsScreen::contextualHelp() const
{
    return tr("AppSettings. "
               "Press Escape to go back.");
}
void AppSettingsScreen::applySettings()
{
auto& profile = m_accessibility->userProfile();
profile.nickname = m_nickname->text().trimmed();
profile.theme    = m_theme->currentIndex();
profile.language = m_language->currentIndex();

    // --- Auto‑Reconnect toggle ---
    bool autoReconnectEnabled = m_autoReconnect->isChecked();

AACModeFlags modes = m_accessibility->modes();
modes.autoReconnect = autoReconnectEnabled;
modes.helpMode = m_helpMode->isChecked();   // ⭐ ADD THIS

if (m_accessibility->speechEngine() && !modes.fatigueMode) {
    m_accessibility->speechEngine()->speakNotification(
        m_helpMode->isChecked()
            ? tr("Help mode enabled")
            : tr("Help mode disabled"),
        SpeechPriority::System
    );
}
m_accessibility->setModes(modes);

    // ⭐ System‑priority announcement
    if (m_accessibility->speechEngine() && !modes.fatigueMode) {
        m_accessibility->speechEngine()->speakNotification(
            autoReconnectEnabled
                ? tr("Auto reconnect enabled")
                : tr("Auto reconnect disabled"),
            SpeechPriority::System
        );
    }
if (m_accessibility->speechEngine() && !modes.fatigueMode) {
    m_accessibility->speechEngine()->speakNotification(
        m_helpMode->isChecked()
            ? tr("Help mode enabled")
            : tr("Help mode disabled"),
        SpeechPriority::System
    );
}

    // --- Persist profile ---
    m_accessibility->saveProfile();

    emit backRequested();
}

// =====================================================
// AACScreenBase overrides
// =====================================================

QVector<QWidget*> AppSettingsScreen::interactiveWidgets() const
{
    return {
        m_nickname,
        m_autoReconnect,
        m_helpMode,
        m_theme,
        m_language,
        m_saveButton,
        m_backButton
    };
}

void AppSettingsScreen::focusFirstInteractive()
{
    if (m_nickname)
        m_nickname->setFocus();
}
QLayout* AppSettingsScreen::rootLayout() const
{
    return layout(); // QVBoxLayout(this)
}