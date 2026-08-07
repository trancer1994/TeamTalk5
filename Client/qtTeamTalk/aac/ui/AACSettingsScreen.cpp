#include "AACSettingsScreen.h"
#include "AACKeyButton.h"
#include "AACToggle.h"
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

    if (m_aac) {
        connect(m_aac, &AACAccessibilityManager::modesChanged,
                this, &AACSettingsScreen::onModesChanged);
    }
}
QString AACSettingsScreen::contextualHelp() const
{
    return tr("AAC Settings. "
               "Press F2 for Speech Settings. "
               "Press F3 for App Settings. "
               "Press Escape to go back.");
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
    //
    // ────────────────────────────────────────────────
    //  ACCESS BLOCK
    // ────────────────────────────────────────────────
    //
    auto* accessBlock = new QVBoxLayout();
    accessBlock->setSpacing(4);
    accessBlock->setContentsMargins(0,0,0,0);

    auto* accessLabel = new QLabel(tr("Access"), this);
    accessLabel->setProperty("aacBlockTitle", true);
    accessBlock->addWidget(accessLabel);

    accessBlock->addWidget(m_largeTargets);
    accessBlock->addWidget(m_dwell);

    // Dwell timing
    accessBlock->addSpacing(4);
    auto* dwellRow = new QHBoxLayout();
    dwellRow->addSpacing(32);
    dwellRow->addWidget(new QLabel(tr("Dwell time (ms)"), this));
    m_dwellTime = new QSlider(Qt::Horizontal, this);
    m_dwellTime->setRange(300, 2000);
    m_dwellTime->setSingleStep(50);
    dwellRow->addWidget(m_dwellTime);
    accessBlock->addLayout(dwellRow);

    accessBlock->addWidget(m_scanning);

// Step scanning toggle
m_stepScanning = new AACToggle(tr("Step scanning (manual)"), this);
accessBlock->addWidget(m_stepScanning);

// Reduce scanning sound intensity
m_reduceScanSound = new AACToggle(tr("Reduce scanning sound intensity"), this);
accessBlock->addWidget(m_reduceScanSound);

    // Scanning speed
    accessBlock->addSpacing(4);
    auto* scanRow = new QHBoxLayout();
    scanRow->addSpacing(32);
    scanRow->addWidget(new QLabel(tr("Scanning speed"), this));
    m_scanningSpeed = new QSlider(Qt::Horizontal, this);
    m_scanningSpeed->setRange(1, 10);
    scanRow->addWidget(m_scanningSpeed);
    accessBlock->addLayout(scanRow);

    // Touch hold delay
    accessBlock->addSpacing(4);
    auto* holdRow = new QHBoxLayout();
    holdRow->addSpacing(32);
    holdRow->addWidget(new QLabel(tr("Touch hold delay"), this));
    m_touchHoldDelay = new QSlider(Qt::Horizontal, this);
    m_touchHoldDelay->setRange(0, 2000);
    holdRow->addWidget(m_touchHoldDelay);
    accessBlock->addLayout(holdRow);

    accessBlock->addWidget(m_oneHand);

    // One-hand side
    accessBlock->addSpacing(4);
    auto* oneHandRow = new QHBoxLayout();
    oneHandRow->addSpacing(32);
    oneHandRow->addWidget(new QLabel(tr("One-hand side"), this));
    m_oneHandSide = new QComboBox(this);
    m_oneHandSide->setStyleSheet(
        "QComboBox { combobox-popup: 0; }"
        "QComboBox QAbstractItemView { animation: none; }"
    );
    m_oneHandSide->addItem(tr("Left"), 0);
    m_oneHandSide->addItem(tr("Right"), 1);
    oneHandRow->addWidget(m_oneHandSide);
    accessBlock->addLayout(oneHandRow);

    accessBlock->addWidget(m_ultraMinimal);

    m_rootLayout->addLayout(accessBlock);
    m_rootLayout->addSpacing(12);


    //
    // ────────────────────────────────────────────────
    //  FEEDBACK BLOCK
    // ────────────────────────────────────────────────
    //
    auto* feedbackBlock = new QVBoxLayout();
    feedbackBlock->setSpacing(4);
    feedbackBlock->setContentsMargins(0,0,0,0);

    auto* feedbackLabel = new QLabel(tr("Feedback"), this);
    feedbackLabel->setProperty("aacBlockTitle", true);
    feedbackBlock->addWidget(feedbackLabel);

m_feedbackEnabled = new AACToggle(tr("Feedback enabled"), this);
feedbackBlock->addWidget(m_feedbackEnabled);
    feedbackBlock->addWidget(m_auditory);
    feedbackBlock->addWidget(m_haptics);

    feedbackBlock->addSpacing(4);
    auto* highlightRow = new QHBoxLayout();
    highlightRow->addWidget(new QLabel(tr("Highlight style"), this));
    m_highlightStyle = new QComboBox(this);
    m_highlightStyle->setStyleSheet(
        "QComboBox { combobox-popup: 0; }"
        "QComboBox QAbstractItemView { animation: none; }"
    );
    m_highlightStyle->addItem(tr("Ring"), 0);
    m_highlightStyle->addItem(tr("Glow"), 1);
    m_highlightStyle->addItem(tr("High contrast"), 2);
    m_highlightStyle->addItem(tr("Invert"), 3);
    highlightRow->addWidget(m_highlightStyle);
    feedbackBlock->addLayout(highlightRow);

    m_rootLayout->addLayout(feedbackBlock);
    m_rootLayout->addSpacing(12);


    //
    // ────────────────────────────────────────────────
    //  PREDICTION BLOCK
    // ────────────────────────────────────────────────
    //
    auto* predictionBlock = new QVBoxLayout();
    predictionBlock->setSpacing(4);
    predictionBlock->setContentsMargins(0,0,0,0);

    auto* predictionLabel = new QLabel(tr("Prediction"), this);
    predictionLabel->setProperty("aacBlockTitle", true);
    predictionBlock->addWidget(predictionLabel);

    predictionBlock->addWidget(m_predictive);
    predictionBlock->addWidget(m_coreSymbolsFirst);
    predictionBlock->addWidget(m_curatedStripDwell);

    m_rootLayout->addLayout(predictionBlock);
    m_rootLayout->addSpacing(12);


    //
    // ────────────────────────────────────────────────
    //  LAYOUT BLOCK
    // ────────────────────────────────────────────────
    //
    auto* layoutBlock = new QVBoxLayout();
    layoutBlock->setSpacing(4);
    layoutBlock->setContentsMargins(0,0,0,0);

    auto* layoutLabel = new QLabel(tr("Layout"), this);
    layoutLabel->setProperty("aacBlockTitle", true);
    layoutBlock->addWidget(layoutLabel);

    layoutBlock->addSpacing(4);
    auto* gridRow = new QHBoxLayout();
    gridRow->addWidget(new QLabel(tr("Grid size"), this));
    m_gridSize = new QComboBox(this);
    m_gridSize->setStyleSheet(
        "QComboBox { combobox-popup: 0; }"
        "QComboBox QAbstractItemView { animation: none; }"
    );
    m_gridSize->addItem(tr("Large"), 0);
    m_gridSize->addItem(tr("Medium"), 1);
    m_gridSize->addItem(tr("Dense"), 2);
    gridRow->addWidget(m_gridSize);
    layoutBlock->addLayout(gridRow);

    m_rootLayout->addLayout(layoutBlock);
    m_rootLayout->addSpacing(12);


//
// ────────────────────────────────────────────────
//  COMMUNICATION BLOCK
// ────────────────────────────────────────────────
//
auto* commBlock = new QVBoxLayout();
commBlock->setSpacing(4);
commBlock->setContentsMargins(0,0,0,0);

auto* commLabel = new QLabel(tr("Communication"), this);
commLabel->setProperty("aacBlockTitle", true);
commBlock->addWidget(commLabel);

// Transmit mode row
auto* txRow = new QHBoxLayout();
txRow->addWidget(new QLabel(tr("Transmit mode"), this));

m_transmitMode = new QComboBox(this);
m_transmitMode->setStyleSheet(
    "QComboBox { combobox-popup: 0; }"
    "QComboBox QAbstractItemView { animation: none; }"
);

m_autoReconnect = new AACToggle(tr("Reconnect automatically"), this);
commBlock->addWidget(m_autoReconnect);
m_speakIncoming = new AACToggle(tr("Speak incoming messages"), this);
commBlock->addWidget(m_speakIncoming);

// Order is AAC‑friendly: simplest → most automatic
m_transmitMode->addItem(tr("Toggle transmit"),      (int)BackendAdapter::AACTransmitMode::TapToToggle);
m_transmitMode->addItem(tr("Continuous"),         (int)BackendAdapter::AACTransmitMode::Continuous);
m_transmitMode->addItem(tr("Voice activation"),   (int)BackendAdapter::AACTransmitMode::VoiceActivation);
m_transmitMode->addItem(tr("Auto‑silence"),       (int)BackendAdapter::AACTransmitMode::AutoSilence);

txRow->addWidget(m_transmitMode);
commBlock->addLayout(txRow);

m_rootLayout->addLayout(commBlock);
m_rootLayout->addSpacing(12);

    //
    // ────────────────────────────────────────────────
    //  ACCESS TEST AREA BLOCK
    // ────────────────────────────────────────────────
    //
    auto* testBlock = new QVBoxLayout();
    testBlock->setSpacing(4);
    testBlock->setContentsMargins(0,0,0,0);

    auto* testLabel = new QLabel(tr("Access Test Area"), this);
    testLabel->setProperty("aacBlockTitle", true);
    testBlock->addWidget(testLabel);

    testBlock->addSpacing(4);
    auto* testRow = new QHBoxLayout();
    for (int i = 0; i < 3; ++i) {
        auto* btn = new AACButton(m_aac, this);
        btn->setText(QString::number(i + 1));
        btn->setMinimumSize(80, 80);
        testRow->addWidget(btn);
        m_interactive.append(btn);
    }
    testBlock->addLayout(testRow);

    m_rootLayout->addLayout(testBlock);
    m_rootLayout->addSpacing(12);


    //
    // ────────────────────────────────────────────────
    //  PRESETS BLOCK
    // ────────────────────────────────────────────────
    //
    auto* presetBlock = new QVBoxLayout();
    presetBlock->setSpacing(4);
    presetBlock->setContentsMargins(0,0,0,0);

    auto* presetLabel = new QLabel(tr("Presets"), this);
    presetLabel->setProperty("aacBlockTitle", true);
    presetBlock->addWidget(presetLabel);

    presetBlock->addWidget(m_presetTouch);
    presetBlock->addWidget(m_presetEyeGaze);
    presetBlock->addWidget(m_presetSwitch);
    presetBlock->addWidget(m_presetCognitiveLow);
    presetBlock->addWidget(m_presetRecommended);

    m_rootLayout->addLayout(presetBlock);
    m_rootLayout->addSpacing(16);


    //
    // ────────────────────────────────────────────────
    //  NAVIGATION
    // ────────────────────────────────────────────────
    //
    m_rootLayout->addWidget(m_resetButton);
    m_rootLayout->addSpacing(8);
    m_rootLayout->addWidget(m_speechSettingsButton);
    m_rootLayout->addSpacing(8);
    m_rootLayout->addWidget(m_backButton);
}

    //
    // ────────────────────────────────────────────────
    //  INTERACTIVE LIST
    // ────────────────────────────────────────────────
    //
    m_interactive << m_largeTargets << m_dwell << m_scanning
              << m_stepScanning << m_reduceScanSound
<< m_feedbackEnabled
                  << m_auditory << m_haptics
                  << m_ultraMinimal << m_oneHand << m_predictive
                  << m_coreSymbolsFirst << m_curatedStripDwell
                << m_autoReconnect
<< m_speakIncoming
                  << m_gridSize << m_highlightStyle
                  << m_scanningSpeed << m_touchHoldDelay << m_dwellTime
                  << m_oneHandSide
                  << m_presetTouch << m_presetEyeGaze << m_presetSwitch
                  << m_presetCognitiveLow << m_presetRecommended
                  << m_resetButton << m_speechSettingsButton << m_backButton;

    //
    // ────────────────────────────────────────────────
    //  CONNECTIONS
    // ────────────────────────────────────────────────
    //
    connect(m_largeTargets,   &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_dwell,          &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_scanning,       &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
connect(m_stepScanning, &AACToggle::toggled,
        this, &AACSettingsScreen::applyToManager);

connect(m_reduceScanSound, &AACToggle::toggled,
        this, &AACSettingsScreen::applyToManager);
connect(m_feedbackEnabled, &AACToggle::toggled,
        this, &AACSettingsScreen::applyToManager);
    connect(m_auditory,       &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_haptics,        &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_ultraMinimal,   &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_oneHand,        &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_predictive,     &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);

    connect(m_coreSymbolsFirst,  &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_curatedStripDwell, &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
connect(m_autoReconnect, &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
connect(m_speakIncoming, &AACToggle::toggled, this, &AACSettingsScreen::applyToManager);
    connect(m_gridSize,       &QComboBox::currentIndexChanged, this, &AACSettingsScreen::onGridSizeChanged);
    connect(m_highlightStyle, &QComboBox::currentIndexChanged, this, &AACSettingsScreen::onHighlightStyleChanged);
    connect(m_scanningSpeed,  &QSlider::valueChanged,          this, &AACSettingsScreen::onScanningSpeedChanged);
    connect(m_touchHoldDelay, &QSlider::valueChanged,          this, &AACSettingsScreen::onTouchHoldDelayChanged);
    connect(m_oneHandSide,    &QComboBox::currentIndexChanged, this, &AACSettingsScreen::applyToManager);

    connect(m_dwellTime, &QSlider::valueChanged,
            this, [this](int v) {
                if (!m_aac)
                    return;
                AACModeFlags m = m_aac->modes();
                m.dwellTimeMs = v;
                m_aac->setModes(m);
            });

connect(m_transmitMode, &QComboBox::currentIndexChanged,
        this, [this](int index) {
            if (!m_aac)
                return;

            // Forward to whoever owns the backend (MainWindow or controller)
            emit transmitModeChanged(index);
        });

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
}

void AACSettingsScreen::onModesChanged(const AACModeFlags& modes)
{
    m_largeTargets->setChecked(modes.largeTargets);
    m_dwell->setChecked(modes.dwell);
    m_scanning->setChecked(modes.scanning);
m_stepScanning->setChecked(modes.stepScanning);
m_reduceScanSound->setChecked(modes.reduceScanningSoundIntensity);
m_feedbackEnabled->setChecked(modes.feedbackEnabled);
    m_auditory->setChecked(modes.auditoryFeedback);
    m_haptics->setChecked(modes.hapticFeedback);
    m_ultraMinimal->setChecked(modes.ultraMinimal);
    m_oneHand->setChecked(modes.oneHandLayout);
    m_predictive->setChecked(modes.predictiveStrip);

    m_coreSymbolsFirst->setChecked(modes.coreSymbolsFirst);
m_autoReconnect->setChecked(modes.autoReconnect);
m_speakIncoming->setChecked(modes.speakIncomingMessages);
    m_curatedStripDwell->setChecked(modes.curatedStripDwell);

    m_dwellTime->setValue(modes.dwellTimeMs);
    // scanningSpeed / touchHoldDelay could be wired to configs later if you expose them
}

void AACSettingsScreen::applyToManager()
{
    if (!m_aac)
        return;

    AACModeFlags modes = m_aac->modes();
    modes.largeTargets      = m_largeTargets->isChecked();
    modes.dwell             = m_dwell->isChecked();
    modes.scanning          = m_scanning->isChecked();
modes.stepScanning = m_stepScanning->isChecked();
modes.reduceScanningSoundIntensity = m_reduceScanSound->isChecked();
modes.feedbackEnabled = m_feedbackEnabled->isChecked();
    modes.auditoryFeedback  = m_auditory->isChecked();
    modes.hapticFeedback    = m_haptics->isChecked();
    modes.ultraMinimal      = m_ultraMinimal->isChecked();
    modes.oneHandLayout     = m_oneHand->isChecked();
    modes.predictiveStrip   = m_predictive->isChecked();

    modes.coreSymbolsFirst  = m_coreSymbolsFirst->isChecked();
modes.autoReconnect = m_autoReconnect->isChecked();
modes.speakIncomingMessages = m_speakIncoming->isChecked();
    modes.curatedStripDwell = m_curatedStripDwell->isChecked();

    m_aac->setModes(modes);
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
    AACModeFlags modes;

    modes.coreSymbolsFirst  = true;
    modes.curatedStripDwell = true;

    applyPreset(modes);
}

void AACSettingsScreen::applyPresetTouch()
{
    AACModeFlags modes;
    modes.largeTargets      = true;
    modes.dwell             = false;
    modes.scanning          = false;
    modes.predictiveStrip   = true;

    modes.coreSymbolsFirst  = false;
    modes.curatedStripDwell = false;

    applyPreset(modes);
}

void AACSettingsScreen::applyPresetEyeGaze()
{
    AACModeFlags modes;
    modes.largeTargets      = true;
    modes.dwell             = true;
    modes.scanning          = false;

    modes.coreSymbolsFirst  = false;
    modes.curatedStripDwell = false;

    applyPreset(modes);
}

void AACSettingsScreen::applyPresetSwitch()
{
    AACModeFlags modes;
    modes.largeTargets      = true;
    modes.dwell             = false;
    modes.scanning          = true;

    modes.coreSymbolsFirst  = true;
    modes.curatedStripDwell = true;

    applyPreset(modes);
}

void AACSettingsScreen::applyPresetCognitiveLow()
{
    AACModeFlags modes;
    modes.largeTargets      = true;
    modes.ultraMinimal      = true;

    modes.coreSymbolsFirst  = true;
    modes.curatedStripDwell = true;

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
