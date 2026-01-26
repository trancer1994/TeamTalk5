#pragma once

#include "AACFramework.h"
#include "AACScreen.h"

class QComboBox;
class QSlider;
class QCheckBox;
class QPushButton;
class QGroupBox;

class AACSpeechSettingsScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACSpeechSettingsScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void applyToManager();
    void previewVoice();
    void toggleExpert(bool enabled);

private:
    // Basic
    QComboBox* m_voiceSelector = nullptr;
    QSlider*   m_rateSlider    = nullptr;
    QSlider*   m_pitchSlider   = nullptr;
    QSlider*   m_volumeSlider  = nullptr;
    QComboBox* m_sayAsType     = nullptr;

    // Advanced
    QCheckBox* m_echoOnSend    = nullptr;
    QCheckBox* m_preTone       = nullptr;
    QCheckBox* m_highIntelligible = nullptr;
    QCheckBox* m_lowIntensity     = nullptr;

    // Expert
    QCheckBox* m_showExpertToggle = nullptr;
    QGroupBox* m_expertGroup      = nullptr;

    // Common
    QPushButton* m_previewButton = nullptr;
    QPushButton* m_backButton    = nullptr;
};
