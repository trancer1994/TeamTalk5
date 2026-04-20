#include "AACSettingsScreen.h"
#include "AACAccessibilityManager.h"
#include "AACButton.h"
#include "AACToggle.h"
#include "AACFramework.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QLabel>

AACSettingsScreen::AACSettingsScreen(AACAccessibilityManager* aac,
                                     QWidget* parent)
    : AACScreenBase(parent)
    , m_aac(aac)
{
    setScreenTitle(tr("AAC Settings"));

    m_rootLayout = new QVBoxLayout();
    m_rootLayout->setContentsMargins(12, 12, 12, 12);
    m_rootLayout->setSpacing(12);

    buildUI();
    syncFromManager();
}

// ------------------------------------------------------------
// AACScreenAdapter overrides
// ------------------------------------------------------------

QList<QWidget*> AACSettingsScreen::interactiveWidgets() const
{
    return m_interactive;
}

QList<QWidget*> AACSettingsScreen::primaryWidgets() const
{
    // All toggles and preset buttons are primary AAC targets.
    return m_interactive;
}

QLayout* AACSettingsScreen::rootLayout() const
{
    return m_rootLayout;
}

// ------------------------------------------------------------
// UI construction
// ------------------------------------------------------------

void AACSettingsScreen::buildUI()
{
    // Core access section
    auto* accessLabel = new QLabel(tr("Access"), this);
    m_rootLayout->addWidget(accessLabel);

    m_largeTargets = new AACToggle(tr("Large targets"), this);
    m_dwell        = new AACToggle(tr("Dwell"), this);
    m_scanning     = new AACToggle(tr("Scanning"), this);
    m_oneHand      = new AACToggle(tr("One-hand mode"), this);
    m_ultraMinimal = new AACToggle(tr("Ultra-minimal mode"), this);

    m_rootLayout->addWidget(m_largeTargets);
    m_rootLayout->addWidget(m_dwell);
    m_rootLayout->addWidget(m_scanning);
    m_rootLayout->addWidget(m_oneHand);
    m_rootLayout->addWidget(m_ultraMinimal);

    // One-hand side
    auto* oneHandRow = new QHBoxLayout();
    oneHandRow->addWidget(new QLabel(tr("One-hand side"), this));
    m_oneHandSide = new QComboBox(this);
    m_oneHandSide->addItem(tr("Left"), 0);
    m_oneHandSide->addItem(tr("Right"), 1);
    oneHandRow->addWidget(m_oneHandSide);
    m_rootLayout->addLayout(oneHandRow);

    // Layout section
    auto* layoutLabel = new QLabel(tr("Layout"), this);
    m_rootLayout->addWidget(layoutLabel);

    auto* gridRow = new QHBoxLayout();
    gridRow->addWidget(new QLabel(tr("Grid size"), this));
    m_gridSize = new QComboBox(this);
    m_gridSize->addItem(tr("Large"), 0);
    m_gridSize->addItem(tr("Medium"), 1);
    m_gridSize->addItem(tr("Dense"), 2);
    gridRow->addWidget(m_gridSize);
    m_rootLayout->addLayout(gridRow);

    // Feedback section
    auto* feedbackLabel = new QLabel(tr("Feedback"), this);
    m_rootLayout->addWidget(feedbackLabel);

    m_auditory = new AACToggle(tr("Auditory feedback"), this);
    m_haptics  = new AACToggle(tr("Haptic feedback"), this);
    m_rootLayout->addWidget(m_auditory);
    m_rootLayout->addWidget(m_haptics);

    auto* highlightRow = new QHBoxLayout();
    highlightRow->addWidget(new QLabel(tr("Highlight style"), this));
    m_highlightStyle = new QComboBox(this);
    m_highlightStyle->addItem(tr("Ring"), 0);
    m_highlightStyle->addItem(tr("Glow"), 1);
    m_highlightStyle->addItem(tr("High contrast"), 2);
    m_highlightStyle->addItem(tr("Invert"), 3);
    highlightRow->addWidget(m_highlightStyle);
    m_rootLayout->addLayout(highlightRow);

    // Prediction section
    auto* predictionLabel = new QLabel(tr("Prediction"), this);
    m_rootLayout->addWidget(predictionLabel);

    m_predictive = new AACToggle(tr("Predictive strip"), this);
    m_rootLayout->addWidget(m_predictive);

    // Scanning / touch timing
    auto* timingLabel = new QLabel(tr("Timing"), this);
    m_rootLayout->addWidget(timingLabel);

    auto* scanRow = new QHBoxLayout();
    scanRow->addWidget(new QLabel(tr("Scanning speed"), this));
    m_scanningSpeed = new QSlider(Qt::Horizontal, this);
    m_scanningSpeed->setRange(1, 10); // abstract units
    scanRow->addWidget(m_scanningSpeed);
    m_rootLayout->addLayout(scanRow);

    auto* holdRow = new QHBoxLayout();
    holdRow->addWidget(new QLabel(tr("Touch hold delay"), this));
    m_touchHoldDelay = new QSlider(Qt::Horizontal, this);
    m_touchHoldDelay->setRange(0, 2000); // ms
    holdRow->addWidget(m_touchHoldDelay);
    m_rootLayout->addLayout(holdRow);

    // Presets
    auto* presetLabel = new QLabel(tr("Presets"), this);
    m_rootLayout->addWidget(presetLabel);

    m_presetTouch        = new AACButton(tr("Touch"), this);
    m_presetEyeGaze      = new AACButton(tr("Eye-gaze"), this);
    m_presetSwitch       = new AACButton(tr("Switch"), this);
    m_presetCognitiveLow = new AACButton(tr("Cognitive-low"), this);
    m_presetRecommended  = new AACButton(tr("Recommended"), this);

    m_rootLayout->addWidget(m_presetTouch);
    m_rootLayout->addWidget(m_presetEyeGaze);
    m_rootLayout->addWidget(m_presetSwitch);
    m_rootLayout->addWidget(m_presetCognitiveLow);
    m_rootLayout->addWidget(m_presetRecommended);

    // Buttons
    m_resetButton          = new AACButton(tr("Reset to defaults"), this);
    m_speechSettingsButton = new AACButton(tr("Speech settings…"), this);
    m_backButton           = new AACButton(tr("Back"), this);

    m_rootLayout->addWidget(m_resetButton);
    m_rootLayout->addWidget(m_speechSettingsButton);
    m_rootLayout->addWidget(m_backButton);

    // Interactive list
    m_interactive = {
        m_largeTargets, m_dwell, m_scanning, m_auditory, m_haptics,
        m_ultraMinimal, m_oneHand, m_predictive,
        m_gridSize, m_highlightStyle,
        m_scanningSpeed, m_touchHoldDelay,
        m_oneHandSide,
        m_presetTouch, m_presetEyeGaze, m_presetSwitch,
        m_presetCognitiveLow, m_presetRecommended,
        m_resetButton, m_speechSettingsButton, m_backButton
    };

    // Connections
    connect(m_largeTargets, &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_dwell,        &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_scanning,     &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_auditory,     &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_haptics,      &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_ultraMinimal, &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_oneHand,      &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_predictive,   &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);

    connect(m_gridSize,       &QComboBox::currentIndexChanged, this, &AACSettingsScreen::onGridSizeChanged);
    connect(m_highlightStyle, &QComboBox::currentIndexChanged, this, &AACSettingsScreen::onHighlightStyleChanged);
    connect(m_scanningSpeed,  &QSlider::valueChanged,          this, &AACSettingsScreen::onScanningSpeedChanged);
    connect(m_touchHoldDelay, &QSlider::valueChanged,          this, &AACSettingsScreen::onTouchHoldDelayChanged);
    connect(m_oneHandSide,    &QComboBox::currentIndexChanged, this, &AACSettingsScreen::applyToManager);

    connect(m_presetTouch,        &AACButton::clicked, this, &AACSettingsScreen::applyPresetTouch);
    connect(m_presetEyeGaze,      &AACButton::clicked, this, &AACSettingsScreen::applyPresetEyeGaze);
    connect(m_presetSwitch,       &AACButton::clicked, this, &AACSettingsScreen::applyPresetSwitch);
    connect(m_presetCognitiveLow, &AACButton::clicked, this, &AACSettingsScreen::applyPresetCognitiveLow);
    connect(m_presetRecommended,  &AACButton::clicked, this, &AACSettingsScreen::applyRecommendedDefaults);

    connect(m_resetButton, &AACButton::clicked, this, &AACSettingsScreen::applyRecommendedDefaults);

    connect(m_speechSettingsButton, &AACButton::clicked,
            this, &AACSettingsScreen::speechSettingsRequested);

    connect(m_backButton, &AACButton::clicked,
            this, &AACSettingsScreen::backRequested);
}

// ------------------------------------------------------------
// Sync with manager
// ------------------------------------------------------------

void AACSettingsScreen::syncFromManager()
{
    if (!m_aac)
        return;

    const AACModeFlags modes = m_aac->modes();
    onModesChanged(modes);

    // Grid size, highlight style, scanning speed, touch hold delay, one-hand side
    // are assumed to be stored in AACAccessibilityManager or related config.
    // Here we just sketch the pattern; you’ll wire to your real fields.

    // Example:
    // m_gridSize->setCurrentIndex(m_aac->gridSize());
    // m_highlightStyle->setCurrentIndex(m_aac->highlightStyle());
    // m_scanningSpeed->setValue(m_aac->scanningSpeed());
    // m_touchHoldDelay->setValue(m_aac->touchHoldDelayMs());
    // m_oneHandSide->setCurrentIndex(m_aac->oneHandSide());
}

void AACSettingsScreen::onModesChanged(const AACModeFlags& modes)
{
    m_largeTargets->setChecked(modes.largeTargets);
    m_dwell->setChecked(modes.dwell);
    m_scanning->setChecked(modes.scanning);
    m_auditory->setChecked(modes.auditoryFeedback);
    m_haptics->setChecked(modes.haptics);
    m_ultraMinimal->setChecked(modes.ultraMinimal);
    m_oneHand->setChecked(modes.oneHand);
    m_predictive->setChecked(modes.predictiveStrip);
}

void AACSettingsScreen::applyToManager()
{
    if (!m_aac)
        return;

    AACModeFlags modes = m_aac->modes();
    modes.largeTargets     = m_largeTargets->isChecked();
    modes.dwell            = m_dwell->isChecked();
    modes.scanning         = m_scanning->isChecked();
    modes.auditoryFeedback = m_auditory->isChecked();
    modes.haptics          = m_haptics->isChecked();
    modes.ultraMinimal     = m_ultraMinimal->isChecked();
    modes.oneHand          = m_oneHand->isChecked();
    modes.predictiveStrip  = m_predictive->isChecked();

    m_aac->setModes(modes);

    // Grid size, highlight style, scanning speed, touch hold delay, one-hand side
    // should also be pushed into your manager here.
}

void AACSettingsScreen::applyPreset(const AACModeFlags& modes)
{
    if (!m_aac)
        return;

    m_aac->setModes(modes);
    onModesChanged(modes);
}

void AACSettingsScreen::applyRecommendedDefaults()
{
    if (!m_aac)
        return;

    AACModeFlags modes;
    // Fill with your recommended defaults.
    applyPreset(modes);
}

void AACSettingsScreen::applyPresetTouch()
{
    AACModeFlags modes;
    // Configure for touch: large targets, no scanning, etc.
    applyPreset(modes);
}

void AACSettingsScreen::applyPresetEyeGaze()
{
    AACModeFlags modes;
    // Configure for eye-gaze: dwell, large targets, etc.
    applyPreset(modes);
}

void AACSettingsScreen::applyPresetSwitch()
{
    AACModeFlags modes;
    // Configure for switch scanning.
    applyPreset(modes);
}

void AACSettingsScreen::applyPresetCognitiveLow()
{
    AACModeFlags modes;
    // Configure for cognitive-low: ultra-minimal, reduced features.
    applyPreset(modes);
}

// ------------------------------------------------------------
// Extra controls
// ------------------------------------------------------------

void AACSettingsScreen::onGridSizeChanged(int index)
{
    Q_UNUSED(index);
    applyToManager();
}

void AACSettingsScreen::onHighlightStyleChanged(int index)
{
    Q_UNUSED(index);
    applyToManager();
}

void AACSettingsScreen::onScanningSpeedChanged(int value)
{
    Q_UNUSED(value);
    applyToManager();
}

void AACSettingsScreen::onTouchHoldDelayChanged(int value)
{
    Q_UNUSED(value);
    applyToManager();
}
