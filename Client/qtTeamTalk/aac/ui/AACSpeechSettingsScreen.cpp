#include "AACSpeechSettingsScreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QTextToSpeech>
#include <QLocale>

AACSpeechSettingsScreen::AACSpeechSettingsScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreen(aac, parent)
{
    auto* layout = new QVBoxLayout(this);

    // -------------------------
    // BASIC SECTION
    // -------------------------
    layout->addWidget(new QLabel(tr("Voice"), this));
    m_voiceSelector = new QComboBox(this);
    layout->addWidget(m_voiceSelector);

    // Local TTS just to enumerate voices / preview
    auto* tts = new QTextToSpeech(this);
    const auto voices = tts->availableVoices();
    for (const QVoice& v : voices) {
        QString label = QStringLiteral("%1 (%2)")
            .arg(v.name())
            .arg(QLocale(v.locale()).nativeLanguageName());
        m_voiceSelector->addItem(label, v.name());
    }

    layout->addWidget(new QLabel(tr("Rate"), this));
    m_rateSlider = new QSlider(Qt::Horizontal, this);
    m_rateSlider->setRange(-100, 100); // -1.0 .. +1.0
    layout->addWidget(m_rateSlider);

    layout->addWidget(new QLabel(tr("Pitch"), this));
    m_pitchSlider = new QSlider(Qt::Horizontal, this);
    m_pitchSlider->setRange(0, 200); // 0.0 .. 2.0
    layout->addWidget(m_pitchSlider);

    layout->addWidget(new QLabel(tr("Volume"), this));
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100); // 0.0 .. 1.0
    layout->addWidget(m_volumeSlider);

    layout->addWidget(new QLabel(tr("Speak as you type"), this));
    m_sayAsType = new QComboBox(this);
    m_sayAsType->addItem(tr("None"), AACSpeechConfig::SpeakNone);
    m_sayAsType->addItem(tr("Letters"), AACSpeechConfig::SpeakLetters);
    m_sayAsType->addItem(tr("Words"), AACSpeechConfig::SpeakWords);
    m_sayAsType->addItem(tr("Phrases"), AACSpeechConfig::SpeakPhrases);
    layout->addWidget(m_sayAsType);

    // -------------------------
    // ADVANCED SECTION
    // -------------------------
    auto* advancedBox = new QGroupBox(tr("Advanced settings"), this);
    advancedBox->setCheckable(true);
    advancedBox->setChecked(false);
    auto* advLayout = new QVBoxLayout();

    m_echoOnSend = new QCheckBox(tr("Echo message on send"), advancedBox);
    m_preTone    = new QCheckBox(tr("Play pre-tone before speech"), advancedBox);
    m_highIntelligible = new QCheckBox(tr("High intelligibility mode"), advancedBox);
    m_lowIntensity     = new QCheckBox(tr("Low intensity mode"), advancedBox);

    advLayout->addWidget(m_echoOnSend);
    advLayout->addWidget(m_preTone);
    advLayout->addWidget(m_highIntelligible);
    advLayout->addWidget(m_lowIntensity);
    advancedBox->setLayout(advLayout);
    layout->addWidget(advancedBox);

    // -------------------------
    // EXPERT SECTION
    // -------------------------
    m_showExpertToggle = new QCheckBox(tr("Show expert voice controls"), this);
    layout->addWidget(m_showExpertToggle);

    m_expertGroup = new QGroupBox(tr("Expert controls"), this);
    m_expertGroup->setVisible(false);
    auto* expertLayout = new QVBoxLayout();
    // Placeholder: future SSML / per-symbol / per-category controls
    expertLayout->addWidget(new QLabel(tr("No expert controls yet – reserved for future features."), m_expertGroup));
    m_expertGroup->setLayout(expertLayout);
    layout->addWidget(m_expertGroup);

    // -------------------------
    // PREVIEW + BACK
    // -------------------------
    m_previewButton = new QPushButton(tr("Preview voice"), this);
    layout->addWidget(m_previewButton);

    m_backButton = new QPushButton(tr("Back"), this);
    layout->addWidget(m_backButton);

    // -------------------------
    // INITIALISE FROM CURRENT CONFIG
    // -------------------------
    if (m_aac) {
        const AACSpeechConfig cfg = m_aac->speechConfig();

        // Voice: select by name if present
        if (!cfg.voiceName.isEmpty()) {
            int idx = m_voiceSelector->findData(cfg.voiceName);
            if (idx < 0)
                idx = m_voiceSelector->findText(cfg.voiceName);
            if (idx >= 0)
                m_voiceSelector->setCurrentIndex(idx);
        }

        m_rateSlider->setValue(int(cfg.rate * 100.0));
        m_pitchSlider->setValue(int(cfg.pitch * 100.0));
        m_volumeSlider->setValue(int(cfg.volume * 100.0));

        int sayIdx = m_sayAsType->findData(int(cfg.speakAsYouTypeMode));
        if (sayIdx >= 0)
            m_sayAsType->setCurrentIndex(sayIdx);

        m_echoOnSend->setChecked(cfg.echoOnSend);
        m_preTone->setChecked(cfg.playPreTone);
        m_highIntelligible->setChecked(cfg.highIntelligible);
        m_lowIntensity->setChecked(cfg.lowIntensity);
    }

    // -------------------------
    // CONNECTIONS
    // -------------------------
    connect(m_previewButton, &QPushButton::clicked,
            this, &AACSpeechSettingsScreen::previewVoice);
    connect(m_backButton, &QPushButton::clicked,
            this, &AACSpeechSettingsScreen::backRequested);

    connect(m_voiceSelector, &QComboBox::currentIndexChanged,
            this, &AACSpeechSettingsScreen::applyToManager);
    connect(m_rateSlider, &QSlider::valueChanged,
            this, &AACSpeechSettingsScreen::applyToManager);
    connect(m_pitchSlider, &QSlider::valueChanged,
            this, &AACSpeechSettingsScreen::applyToManager);
    connect(m_volumeSlider, &QSlider::valueChanged,
            this, &AACSpeechSettingsScreen::applyToManager);
    connect(m_sayAsType, &QComboBox::currentIndexChanged,
            this, &AACSpeechSettingsScreen::applyToManager);
    connect(m_echoOnSend, &QCheckBox::toggled,
            this, &AACSpeechSettingsScreen::applyToManager);
    connect(m_preTone, &QCheckBox::toggled,
            this, &AACSpeechSettingsScreen::applyToManager);
    connect(m_highIntelligible, &QCheckBox::toggled,
            this, &AACSpeechSettingsScreen::applyToManager);
    connect(m_lowIntensity, &QCheckBox::toggled,
            this, &AACSpeechSettingsScreen::applyToManager);

    connect(m_showExpertToggle, &QCheckBox::toggled,
            this, &AACSpeechSettingsScreen::toggleExpert);
}

void AACSpeechSettingsScreen::applyToManager()
{
    if (!m_aac)
        return;

    AACSpeechConfig cfg = m_aac->speechConfig();

    cfg.voiceName = m_voiceSelector->currentData().toString();
    if (cfg.voiceName.isEmpty())
        cfg.voiceName = m_voiceSelector->currentText();

    cfg.rate   = m_rateSlider->value() / 100.0;
    cfg.pitch  = m_pitchSlider->value() / 100.0;
    cfg.volume = m_volumeSlider->value() / 100.0;

    cfg.speakAsYouTypeMode =
        static_cast<AACSpeechConfig::SpeakAsYouTypeMode>(m_sayAsType->currentData().toInt());

    cfg.echoOnSend       = m_echoOnSend->isChecked();
    cfg.playPreTone      = m_preTone->isChecked();
    cfg.highIntelligible = m_highIntelligible->isChecked();
    cfg.lowIntensity     = m_lowIntensity->isChecked();

    m_aac->setSpeechConfig(cfg);
}

void AACSpeechSettingsScreen::previewVoice()
{
    if (!m_aac || !m_aac->speechEngine())
        return;

    // Use the current config so preview matches what will be used
    applyToManager();
    m_aac->speechEngine()->speak(tr("This is my voice."));
}

void AACSpeechSettingsScreen::toggleExpert(bool enabled)
{
    if (m_expertGroup)
        m_expertGroup->setVisible(enabled);
}
