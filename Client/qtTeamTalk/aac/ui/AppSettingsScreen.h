#pragma once

#include "AACScreenBase.h"

class QLineEdit;
class QSpinBox;
class QComboBox;

class AACButton;
class AACToggle;

// Top-level App Settings screen (AAC-native):
// - Nickname
// - Auto-reconnect
// - Theme
// - Language
class AppSettingsScreen : public AACScreenBase {
    Q_OBJECT
public:
    explicit AppSettingsScreen(AACAccessibilityManager* aac,
                               QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void applySettings();

private:
    // --- Interactive fields ---
    QLineEdit*  m_nickname      = nullptr;

    AACToggle*  m_autoReconnect = nullptr;
AACToggle* m_helpMode = nullptr;

    QComboBox*  m_theme         = nullptr;
    QComboBox*  m_language      = nullptr;

    // --- Action buttons ---
    AACButton*  m_saveButton    = nullptr;
    AACButton*  m_backButton    = nullptr;

    // --- AACScreenBase overrides ---
    QVector<QWidget*> interactiveWidgets() const override;
    void focusFirstInteractive() override;
    QLayout* rootLayout() const override;   // ⭐ ADD THIS LINE
};
