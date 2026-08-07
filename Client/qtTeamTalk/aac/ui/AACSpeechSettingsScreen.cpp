#include "AACSpeechSettingsScreen.h"
#include "AACKeyButton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QClipboard>
#include <QGuiApplication>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

// ============================================================
// Utility
// ============================================================

static QString settingsPathForProfile(const QString &profileId)
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return base + "/aacspeech_" + profileId + ".ini";
}

static QString exportPathForProfile(const QString &profileId)
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir().mkpath(base);
    return base + "/aacspeech_profile_" + profileId + ".json";
}

// ============================================================
// Block Implementations (flat, AAC‑native)
// ============================================================

// ---------------- Identity Block ----------------

AACSpeechIdentityBlock::AACSpeechIdentityBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Voice identity"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    auto *form = new QFormLayout();
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(4);

    m_voiceNameLabel = new QLabel("-", this);
    m_localeLabel    = new QLabel("-", this);
    m_engineLabel    = new QLabel("-", this);
    m_categoryLabel  = new QLabel("-", this);
    m_regionalLabel  = new QLabel("-", this);

    m_notesEdit = new QTextEdit(this);
m_notesEdit->setInputMethodHints(Qt::ImhNoPredictiveText);
m_notesEdit->setAcceptRichText(false);
m_notesEdit->setFixedHeight(80);
m_notesEdit->setUndoRedoEnabled(false);
    m_notesEdit->setPlaceholderText(tr("Identity notes…"));

    form->addRow(tr("Voice:"),   m_voiceNameLabel);
    form->addRow(tr("Locale:"),  m_localeLabel);
    form->addRow(tr("Engine:"),  m_engineLabel);
    form->addRow(tr("Category:"),m_categoryLabel);
    form->addRow(tr("Regional:"),m_regionalLabel);
    form->addRow(tr("Notes:"),   m_notesEdit);

    layout->addLayout(form);
}

void AACSpeechIdentityBlock::setCurrentVoiceInfo(const QString &name,
                                                 const QString &locale,
                                                 const QString &engine,
                                                 const QString &category)
{
    m_voiceNameLabel->setText(name);
    m_localeLabel->setText(locale);
    m_engineLabel->setText(engine);
    m_categoryLabel->setText(category);
}

void AACSpeechIdentityBlock::setRegionalInfo(const QStringList &locales)
{
    m_regionalLabel->setText(locales.join(", "));
}

void AACSpeechIdentityBlock::setIdentityNotes(const QString &notes)
{
    m_notesEdit->setPlainText(notes);
}

QString AACSpeechIdentityBlock::identityNotes() const
{
    return m_notesEdit->toPlainText();
}

// ---------------- Presets Block ----------------

AACSpeechPresetsBlock::AACSpeechPresetsBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Presets"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_presetCombo = new QComboBox(this);
m_presetCombo->setStyleSheet(
    "QComboBox { combobox-popup: 0; }"
    "QComboBox QAbstractItemView { animation: none; }"
);
    m_presetDescription = new QLabel("-", this);
    m_presetDescription->setWordWrap(true);

    layout->addWidget(m_presetCombo);
    layout->addWidget(m_presetDescription);

    connect(m_presetCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onPresetComboChanged(int)));
}

void AACSpeechPresetsBlock::populatePresets(const AACPresetEngine &engine)
{
    m_presetCombo->clear();
    for (int i = 0; i < engine.presetCount(); ++i)
        m_presetCombo->addItem(engine.presetAt(i).label);
}

void AACSpeechPresetsBlock::setCurrentPresetIndex(int index)
{
    m_presetCombo->setCurrentIndex(index);
}

int AACSpeechPresetsBlock::currentPresetIndex() const
{
    return m_presetCombo->currentIndex();
}

void AACSpeechPresetsBlock::onPresetComboChanged(int index)
{
    emit presetChanged(index);
}

// ---------------- Voice Block ----------------

AACSpeechVoiceBlock::AACSpeechVoiceBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Voice selection"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_voiceCombo = new QComboBox(this);
m_voiceCombo->setStyleSheet(
    "QComboBox { combobox-popup: 0; }"
    "QComboBox QAbstractItemView { animation: none; }"
);
    m_stabilityLabel = new QLabel("-", this);
    m_safeModeLabel = new QLabel("-", this);
    m_safeModeLabel->setStyleSheet("color: red; font-weight: bold;");
    m_recheckButton = new AACButton(tr("Recheck voices"), this);

    layout->addWidget(m_voiceCombo);
    layout->addWidget(m_stabilityLabel);
    layout->addWidget(m_safeModeLabel);
    layout->addWidget(m_recheckButton);

    connect(m_voiceCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onVoiceComboChanged(int)));
    connect(m_recheckButton, &AACButton::clicked,
            this, &AACSpeechVoiceBlock::onRecheckClicked);
}

void AACSpeechVoiceBlock::setVoices(const QVector<AACVoiceResolver::VoiceInfo> &infos)
{
    m_voiceCombo->clear();
    for (const auto &info : infos)
        m_voiceCombo->addItem(info.label);
}

void AACSpeechVoiceBlock::setHasVoices(bool has)
{
    if (!has)
        m_safeModeLabel->setText(tr("No voices available"));
    else
        m_safeModeLabel->clear();
}

void AACSpeechVoiceBlock::setCurrentVoiceIndex(int index)
{
    m_voiceCombo->setCurrentIndex(index);
}

int AACSpeechVoiceBlock::currentVoiceIndex() const
{
    return m_voiceCombo->currentIndex();
}

void AACSpeechVoiceBlock::setStabilityInfo(int voiceCount, const QString &lastChange)
{
    m_stabilityLabel->setText(
        tr("Voices: %1 | Last change: %2").arg(voiceCount).arg(lastChange));
}

void AACSpeechVoiceBlock::setSafeMode(bool safeMode, const QString &detail)
{
    if (safeMode)
        m_safeModeLabel->setText(detail);
    else
        m_safeModeLabel->clear();
}

void AACSpeechVoiceBlock::setLocked(bool locked)
{
    m_locked = locked;
    m_voiceCombo->setEnabled(!locked);
}

void AACSpeechVoiceBlock::onVoiceComboChanged(int index)
{
    if (m_locked) {
        m_voiceCombo->blockSignals(true);
        m_voiceCombo->setCurrentIndex(m_voiceCombo->currentIndex());
        m_voiceCombo->blockSignals(false);
        return;
    }
    emit voiceChanged(index);
}

void AACSpeechVoiceBlock::onRecheckClicked()
{
    emit recheckRequested();
}

// ---------------- Shaping Block ----------------

AACSpeechShapingBlock::AACSpeechShapingBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Shaping"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_rateSlider = new QSlider(Qt::Horizontal, this);
    m_pitchSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider = new QSlider(Qt::Horizontal, this);

    m_rateSlider->setRange(-40, 40);
    m_pitchSlider->setRange(-30, 30);
    m_volumeSlider->setRange(0, 100);

    auto *form = new QFormLayout();
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(4);
    form->addRow(tr("Rate:"),   m_rateSlider);
    form->addRow(tr("Pitch:"),  m_pitchSlider);
    form->addRow(tr("Volume:"), m_volumeSlider);

    layout->addLayout(form);

    m_previewButton       = new AACButton(tr("Preview"), this);
    m_previewSlowButton   = new AACButton(tr("Preview slow"), this);
    m_previewMediumButton = new AACButton(tr("Preview medium"), this);
    m_previewFastButton   = new AACButton(tr("Preview fast"), this);

    layout->addWidget(m_previewButton);
    layout->addWidget(m_previewSlowButton);
    layout->addWidget(m_previewMediumButton);
    layout->addWidget(m_previewFastButton);

    connect(m_rateSlider,  SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged()));
    connect(m_pitchSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged()));
    connect(m_volumeSlider,SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged()));

    connect(m_previewButton,       &AACButton::clicked, this, &AACSpeechShapingBlock::onPreviewClicked);
    connect(m_previewSlowButton,   &AACButton::clicked, this, &AACSpeechShapingBlock::onPreviewSpeedClicked);
    connect(m_previewMediumButton, &AACButton::clicked, this, &AACSpeechShapingBlock::onPreviewSpeedClicked);
    connect(m_previewFastButton,   &AACButton::clicked, this, &AACSpeechShapingBlock::onPreviewSpeedClicked);
}

void AACSpeechShapingBlock::setRate(int v)   { m_rateSlider->setValue(v); }
void AACSpeechShapingBlock::setPitch(int v)  { m_pitchSlider->setValue(v); }
void AACSpeechShapingBlock::setVolume(int v) { m_volumeSlider->setValue(v); }

int AACSpeechShapingBlock::rate() const   { return m_rateSlider->value(); }
int AACSpeechShapingBlock::pitch() const  { return m_pitchSlider->value(); }
int AACSpeechShapingBlock::volume() const { return m_volumeSlider->value(); }

void AACSpeechShapingBlock::onSliderChanged()
{
    emit shapingChanged();
}

void AACSpeechShapingBlock::onPreviewClicked()
{
    emit previewRequested(
        AACPresetEngine::mapRateToTts(rate()),
        AACPresetEngine::mapPitchToTts(pitch()),
        AACPresetEngine::mapVolumeToTts(volume()),
        0);
}

void AACSpeechShapingBlock::onPreviewSpeedClicked()
{
    QObject *s = sender();
    int preset = 0;
    if (s == m_previewSlowButton)      preset = -1;
    else if (s == m_previewMediumButton) preset = 0;
    else if (s == m_previewFastButton) preset = +1;

    emit previewRequested(
        AACPresetEngine::mapRateToTts(rate()),
        AACPresetEngine::mapPitchToTts(pitch()),
        AACPresetEngine::mapVolumeToTts(volume()),
        preset);
}

// ---------------- Typing Block ----------------

AACSpeechTypingBlock::AACSpeechTypingBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Typing behaviour"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_modeCombo = new QComboBox(this);
m_modeCombo->setStyleSheet(
    "QComboBox { combobox-popup: 0; }"
    "QComboBox QAbstractItemView { animation: none; }"
);
    m_modeCombo->addItem(tr("Off"));
    m_modeCombo->addItem(tr("Word"));
    m_modeCombo->addItem(tr("Sentence"));

    m_fatigueToggle = new AACToggle(tr("Fatigue mode"), this);

    layout->addWidget(m_modeCombo);
    layout->addWidget(m_fatigueToggle);

    connect(m_modeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onModeChanged(int)));
    connect(m_fatigueToggle, &AACToggle::toggled,
            this, &AACSpeechTypingBlock::onFatigueChanged);
}

void AACSpeechTypingBlock::setMode(SpeakAsYouTypeMode mode)
{
    m_modeCombo->setCurrentIndex(static_cast<int>(mode));
}

SpeakAsYouTypeMode AACSpeechTypingBlock::mode() const
{
    return static_cast<SpeakAsYouTypeMode>(m_modeCombo->currentIndex());
}

void AACSpeechTypingBlock::setFatigueMode(bool on)
{
    m_fatigueToggle->setChecked(on);
}

bool AACSpeechTypingBlock::fatigueMode() const
{
    return m_fatigueToggle->isChecked();
}

void AACSpeechTypingBlock::onModeChanged(int index)
{
    emit typingModeChanged(static_cast<SpeakAsYouTypeMode>(index));
}

void AACSpeechTypingBlock::onFatigueChanged(bool on)
{
    emit fatigueModeChanged(on);
}

// ---------------- Pronunciation Block ----------------

AACSpeechPronunciationBlock::AACSpeechPronunciationBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Pronunciation overrides"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_table = new QTableWidget(5, 2, this);
    m_table->setHorizontalHeaderLabels({tr("Word"), tr("Pronunciation")});
    m_table->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(m_table);
}

void AACSpeechPronunciationBlock::setEntries(const QList<QPair<QString, QString>> &entries)
{
    m_table->clearContents();
    for (int i = 0; i < entries.size() && i < m_table->rowCount(); ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(entries[i].first));
        m_table->setItem(i, 1, new QTableWidgetItem(entries[i].second));
    }
}

QList<QPair<QString, QString>> AACSpeechPronunciationBlock::entries() const
{
    QList<QPair<QString, QString>> out;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        auto *w = m_table->item(i, 0);
        auto *p = m_table->item(i, 1);
        if (w && p && !w->text().isEmpty())
            out.append({w->text(), p->text()});
    }
    return out;
}

// ---------------- Phrases Block ----------------

AACSpeechPhrasesBlock::AACSpeechPhrasesBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Quick phrases"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_identityBtn        = new AACButton(tr("My name is…"), this);
    m_assistBtn          = new AACButton(tr("I need assistance"), this);
    m_waitBtn            = new AACButton(tr("Please wait"), this);
    m_emergencyHelpBtn   = new AACButton(tr("I need help"), this);
    m_emergencyDangerBtn = new AACButton(tr("This is an emergency"), this);

    layout->addWidget(m_identityBtn);
    layout->addWidget(m_assistBtn);
    layout->addWidget(m_waitBtn);
    layout->addWidget(m_emergencyHelpBtn);
    layout->addWidget(m_emergencyDangerBtn);

    connect(m_identityBtn,        &AACButton::clicked, this, &AACSpeechPhrasesBlock::onIdentityPhraseClicked);
    connect(m_assistBtn,          &AACButton::clicked, this, &AACSpeechPhrasesBlock::onAssistPhraseClicked);
    connect(m_waitBtn,            &AACButton::clicked, this, &AACSpeechPhrasesBlock::onPleaseWaitPhraseClicked);
    connect(m_emergencyHelpBtn,   &AACButton::clicked, this, &AACSpeechPhrasesBlock::onEmergencyHelpClicked);
    connect(m_emergencyDangerBtn, &AACButton::clicked, this, &AACSpeechPhrasesBlock::onEmergencyDangerClicked);
}

void AACSpeechPhrasesBlock::onIdentityPhraseClicked()
{
    emit previewPhraseRequested(tr("My name is…"));
}

void AACSpeechPhrasesBlock::onAssistPhraseClicked()
{
    emit previewPhraseRequested(tr("I need assistance."));
}

void AACSpeechPhrasesBlock::onPleaseWaitPhraseClicked()
{
    emit previewPhraseRequested(tr("Please wait."));
}

void AACSpeechPhrasesBlock::onEmergencyHelpClicked()
{
    emit previewPhraseRequested(tr("I need help."));
}

void AACSpeechPhrasesBlock::onEmergencyDangerClicked()
{
    emit previewPhraseRequested(tr("This is an emergency."));
}

// ---------------- History + Recovery Block ----------------

AACSpeechHistoryRecoveryBlock::AACSpeechHistoryRecoveryBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("History & recovery"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_historyCombo = new QComboBox(this);
m_historyCombo->setStyleSheet(
    "QComboBox { combobox-popup: 0; }"
    "QComboBox QAbstractItemView { animation: none; }"
);
    m_lockToggle   = new AACToggle(tr("Lock voice identity"), this);
    m_lockTimerCombo = new QComboBox(this);
m_lockTimerCombo->setStyleSheet(
    "QComboBox { combobox-popup: 0; }"
    "QComboBox QAbstractItemView { animation: none; }"
);
    m_lockTimerCombo->addItem(tr("No timer"), 0);
    m_lockTimerCombo->addItem(tr("5 minutes"), 5);
    m_lockTimerCombo->addItem(tr("15 minutes"), 15);
    m_lockTimerCombo->addItem(tr("30 minutes"), 30);
    m_lockTimerCombo->addItem(tr("60 minutes"), 60);

    m_recoverButton = new AACButton(tr("Recover voice"), this);

    layout->addWidget(m_historyCombo);
    layout->addWidget(m_lockToggle);
    layout->addWidget(m_lockTimerCombo);
    layout->addWidget(m_recoverButton);

    connect(m_historyCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onHistoryChanged(int)));
    connect(m_lockToggle, &AACToggle::toggled,
            this, &AACSpeechHistoryRecoveryBlock::onLockChanged);
    connect(m_lockTimerCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onLockTimerChanged(int)));
    connect(m_recoverButton, &AACButton::clicked,
            this, &AACSpeechHistoryRecoveryBlock::onRecoverClicked);
}

void AACSpeechHistoryRecoveryBlock::setHistory(const QStringList &items)
{
    m_historyCombo->clear();
    m_historyCombo->addItems(items);
}

void AACSpeechHistoryRecoveryBlock::setLockEnabled(bool locked)
{
    m_lockToggle->setChecked(locked);
}

bool AACSpeechHistoryRecoveryBlock::lockEnabled() const
{
    return m_lockToggle->isChecked();
}

void AACSpeechHistoryRecoveryBlock::setLockTimerMinutes(int minutes)
{
    for (int i = 0; i < m_lockTimerCombo->count(); ++i) {
        if (m_lockTimerCombo->itemData(i).toInt() == minutes) {
            m_lockTimerCombo->setCurrentIndex(i);
            return;
        }
    }
}

int AACSpeechHistoryRecoveryBlock::lockTimerMinutes() const
{
    return m_lockTimerCombo->currentData().toInt();
}

void AACSpeechHistoryRecoveryBlock::onHistoryChanged(int index)
{
    if (index >= 0)
        emit historyItemSelected(index);
}

void AACSpeechHistoryRecoveryBlock::onLockChanged(bool on)
{
    emit lockChanged(on);
}

void AACSpeechHistoryRecoveryBlock::onLockTimerChanged(int index)
{
    emit lockTimerChanged(m_lockTimerCombo->itemData(index).toInt());
}

void AACSpeechHistoryRecoveryBlock::onRecoverClicked()
{
    emit recoveryRequested();
}

// ---------------- Export Block ----------------

AACSpeechExportBlock::AACSpeechExportBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Export / import"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_exportButton   = new AACButton(tr("Export JSON profile"), this);
    m_importButton   = new AACButton(tr("Import JSON profile"), this);
    m_snapshotButton = new AACButton(tr("Copy snapshot to clipboard"), this);
    m_qrButton       = new AACButton(tr("Copy QR payload text"), this);

    layout->addWidget(m_exportButton);
    layout->addWidget(m_importButton);
    layout->addWidget(m_snapshotButton);
    layout->addWidget(m_qrButton);

    connect(m_exportButton,   &AACButton::clicked, this, &AACSpeechExportBlock::onExportClicked);
    connect(m_importButton,   &AACButton::clicked, this, &AACSpeechExportBlock::onImportClicked);
    connect(m_snapshotButton, &AACButton::clicked, this, &AACSpeechExportBlock::onSnapshotClicked);
    connect(m_qrButton,       &AACButton::clicked, this, &AACSpeechExportBlock::onQrClicked);
}

void AACSpeechExportBlock::onExportClicked()
{
    emit exportRequested();
}

void AACSpeechExportBlock::onImportClicked()
{
    emit importRequested();
}

void AACSpeechExportBlock::onSnapshotClicked()
{
    emit snapshotRequested();
}

void AACSpeechExportBlock::onQrClicked()
{
    emit qrRequested();
}

// ---------------- Analytics Block ----------------

AACSpeechAnalyticsBlock::AACSpeechAnalyticsBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Local analytics"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    auto *form = new QFormLayout();
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(4);

    m_presetLabel = new QLabel("-", this);
    m_voiceLabel  = new QLabel("-", this);
    m_rateLabel   = new QLabel("-", this);
    m_pitchLabel  = new QLabel("-", this);
    m_volumeLabel = new QLabel("-", this);

    form->addRow(tr("Most used preset:"), m_presetLabel);
    form->addRow(tr("Most used voice:"),  m_voiceLabel);
    form->addRow(tr("Average rate:"),     m_rateLabel);
    form->addRow(tr("Average pitch:"),    m_pitchLabel);
    form->addRow(tr("Average volume:"),   m_volumeLabel);

    layout->addLayout(form);
}

void AACSpeechAnalyticsBlock::setMostUsedPreset(const QString &id)
{
    m_presetLabel->setText(id.isEmpty() ? "-" : id);
}

void AACSpeechAnalyticsBlock::setMostUsedVoice(const QString &name)
{
    m_voiceLabel->setText(name.isEmpty() ? "-" : name);
}

void AACSpeechAnalyticsBlock::setAverageRate(int v)
{
    m_rateLabel->setText(QString::number(v));
}

void AACSpeechAnalyticsBlock::setAveragePitch(int v)
{
    m_pitchLabel->setText(QString::number(v));
}

void AACSpeechAnalyticsBlock::setAverageVolume(int v)
{
    m_volumeLabel->setText(QString::number(v));
}

// ---------------- Reset Block ----------------

AACSpeechResetBlock::AACSpeechResetBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Reset"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_resetButton = new AACButton(tr("AAC-safe reset"), this);
    layout->addWidget(m_resetButton);

    connect(m_resetButton, &AACButton::clicked,
            this, &AACSpeechResetBlock::onResetClicked);
}

void AACSpeechResetBlock::onResetClicked()
{
    emit resetRequested();
}

// ---------------- Status Block ----------------

AACSpeechStatusBlock::AACSpeechStatusBlock(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("Speech engine status"), this);
    title->setProperty("aacBlockTitle", true);
    layout->addWidget(title);

    m_statusLabel = new QLabel("-", this);
    m_detailLabel = new QLabel("-", this);
    m_detailLabel->setWordWrap(true);

    layout->addWidget(m_statusLabel);
    layout->addWidget(m_detailLabel);
}

void AACSpeechStatusBlock::setStatusText(const QString &text)
{
    m_statusLabel->setText(text);
}

void AACSpeechStatusBlock::setDetailText(const QString &text)
{
    m_detailLabel->setText(text);
}

// ============================================================
// Helper classes (AACVoiceResolver, AACPresetEngine, AACSpeechSafety)
// ============================================================

void AACVoiceResolver::setVoices(const QVector<QVoice> &voices)
{
    m_voiceInfos.clear();
    for (const QVoice &v : voices) {
        VoiceInfo info;
        info.voice = v;
        QStringList parts;
        parts << v.name();
        parts << v.locale().name();
        parts << QVoice::genderName(v.gender());
        parts << QVoice::ageName(v.age());
        info.label = parts.join(" | ");
        m_voiceInfos.push_back(info);
    }
}

int AACVoiceResolver::findByNameFragment(const QString &fragment) const
{
    if (fragment.isEmpty())
        return -1;
    const QString f = fragment.toLower();
    for (int i = 0; i < m_voiceInfos.size(); ++i) {
        if (m_voiceInfos[i].voice.name().toLower().contains(f))
            return i;
    }
    return -1;
}

int AACVoiceResolver::findClosestByLocaleHint(QLocale::Language lang,
                                              QLocale::Country country) const
{
    int best = -1;
    for (int i = 0; i < m_voiceInfos.size(); ++i) {
        const QLocale loc = m_voiceInfos[i].voice.locale();
        if (loc.language() == lang) {
            if (country == QLocale::AnyCountry || loc.country() == country)
                return i;
            if (best < 0)
                best = i;
        }
    }
    return best;
}

namespace {
static const AACPresetEngine::Preset kPresets[] = {
    { "identity_neutral", "Neutral identity", "", 0, 0, 80, "identity",
      "Balanced rate and pitch for everyday use." },
    { "identity_calm", "Calm / soft", "", -10, -5, 70, "identity",
      "Softer, slightly slower voice for low-fatigue use." },
    { "identity_bright", "Bright / clear", "", 10, 5, 85, "identity",
      "Slightly faster and brighter for clearer articulation." },
    { "regional_uk", "Regional: UK English", "UK", 0, 0, 80, "regional",
      "Prefers UK English voices when available." },
    { "regional_us", "Regional: US English", "US", 0, 0, 80, "regional",
      "Prefers US English voices when available." },
    { "style_reading", "Style: Reading", "", -5, -2, 80, "style",
      "Slightly slower for reading longer text." },
    { "style_announcement", "Style: Announcement", "", 15, 5, 90, "style",
      "Faster and louder for short announcements." },
    { "fatigue_low", "Fatigue: Low effort", "", -15, -5, 75, "fatigue",
      "Lower rate and pitch to reduce listening effort." }
};
}

AACPresetEngine::AACPresetEngine()
{
    m_presets = kPresets;
    m_presetCount = int(sizeof(kPresets) / sizeof(kPresets[0]));
}

int AACPresetEngine::indexForId(const QString &id) const
{
    for (int i = 0; i < m_presetCount; ++i) {
        if (id == m_presets[i].id)
            return i;
    }
    return -1;
}

double AACPresetEngine::mapRateToTts(int rate)
{
    return 1.0 + (rate / 40.0) * 0.5;
}

double AACPresetEngine::mapPitchToTts(int pitch)
{
    return 1.0 + (pitch / 30.0) * 0.3;
}

double AACPresetEngine::mapVolumeToTts(int volume)
{
    return qBound(0.0, volume / 100.0, 1.0);
}

QString AACSpeechSafety::statusForState(QTextToSpeech::State state,
                                        bool hasVoices) const
{
    if (!hasVoices)
        return QStringLiteral("No voices available");
    switch (state) {
    case QTextToSpeech::Ready:    return QStringLiteral("Ready");
    case QTextToSpeech::Speaking: return QStringLiteral("Speaking");
    case QTextToSpeech::Paused:   return QStringLiteral("Paused");
    case QTextToSpeech::Error:    return QStringLiteral("Error");
    default:                      return QStringLiteral("Unknown");
    }
}

QString AACSpeechSafety::noVoicesDetail() const
{
    return QStringLiteral("No text-to-speech voices were found. "
                          "Check your operating system speech settings.");
}

QString AACSpeechSafety::engineErrorDetail() const
{
    return QStringLiteral("The speech engine reported an error. "
                          "Try another voice or check system speech settings.");
}

// ============================================================
// AACSpeechSettingsScreen
// ============================================================

AACSpeechSettingsScreen::AACSpeechSettingsScreen(QWidget *parent)
    : AACScreenBase(parent)
{
    setScreenTitle(tr("Speech settings"));

    m_rootLayout = new QVBoxLayout();
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    setupUi();
    initTts();
    reloadVoices();
    loadSettings();
    connectSignals();
    updateIdentityBlock();
    updateStatusBlock();
    updateAnalyticsBlock();
    updateHistoryBlock();
}

AACSpeechSettingsScreen::~AACSpeechSettingsScreen()
{
    saveSettings();
    delete m_tts;
}

void AACSpeechSettingsScreen::setProfileId(const QString &profileId)
{
    if (m_profileId == profileId)
        return;
    saveSettings();
    m_profileId = profileId;
    loadSettings();
    updateAnalyticsBlock();
    updateHistoryBlock();
}

SpeakAsYouTypeMode AACSpeechSettingsScreen::speakAsYouTypeMode() const
{
    return m_typingBlock->mode();
}

bool AACSpeechSettingsScreen::fatigueMode() const
{
    return m_typingBlock->fatigueMode();
}

// AACScreenAdapter overrides

QList<QWidget*> AACSpeechSettingsScreen::interactiveWidgets() const
{
    return m_interactive;
}

QList<QWidget*> AACSpeechSettingsScreen::primaryWidgets() const
{
    return m_interactive;
}

QLayout* AACSpeechSettingsScreen::rootLayout() const
{
    return m_rootLayout;
}

void AACSpeechSettingsScreen::setupUi()
{
    m_identityBlock      = new AACSpeechIdentityBlock(this);
    m_presetsBlock       = new AACSpeechPresetsBlock(this);
    m_voiceBlock         = new AACSpeechVoiceBlock(this);
    m_shapingBlock       = new AACSpeechShapingBlock(this);
    m_typingBlock        = new AACSpeechTypingBlock(this);
    m_pronunciationBlock = new AACSpeechPronunciationBlock(this);
    m_phrasesBlock       = new AACSpeechPhrasesBlock(this);
    m_historyBlock       = new AACSpeechHistoryRecoveryBlock(this);
    m_exportBlock        = new AACSpeechExportBlock(this);
    m_analyticsBlock     = new AACSpeechAnalyticsBlock(this);
    m_resetBlock         = new AACSpeechResetBlock(this);
    m_statusBlock        = new AACSpeechStatusBlock(this);

    m_rootLayout->addWidget(m_identityBlock);
    m_rootLayout->addWidget(m_presetsBlock);
    m_rootLayout->addWidget(m_voiceBlock);
    m_rootLayout->addWidget(m_shapingBlock);
    m_rootLayout->addWidget(m_typingBlock);
    m_rootLayout->addWidget(m_pronunciationBlock);
    m_rootLayout->addWidget(m_phrasesBlock);
    m_rootLayout->addWidget(m_historyBlock);
    m_rootLayout->addWidget(m_exportBlock);
    m_rootLayout->addWidget(m_analyticsBlock);
    m_rootLayout->addWidget(m_resetBlock);
    m_rootLayout->addWidget(m_statusBlock);

    m_presetsBlock->populatePresets(m_presetEngine);

    // coarse interactive list; you can refine to child widgets later
    m_interactive = {
        m_presetsBlock,
        m_voiceBlock,
        m_shapingBlock,
        m_typingBlock,
        m_pronunciationBlock,
        m_phrasesBlock,
        m_historyBlock,
        m_exportBlock,
        m_resetBlock
    };
}

void AACSpeechSettingsScreen::initTts()
{
    m_tts = new QTextToSpeech(this);
    connect(m_tts, &QTextToSpeech::stateChanged,
            this, &AACSpeechSettingsScreen::onTtsStateChanged);
}

QString AACSpeechSettingsScreen::contextualHelp() const
{
    return tr("Speech Settings. "
               "Press F1 for AAC Settings. "
               "Press Escape to go back.");
}
void AACSpeechSettingsScreen::connectSignals()
{
    connect(m_presetsBlock, &AACSpeechPresetsBlock::presetChanged,
            this, &AACSpeechSettingsScreen::onPresetChanged);
    connect(m_voiceBlock, &AACSpeechVoiceBlock::voiceChanged,
            this, &AACSpeechSettingsScreen::onVoiceChanged);
    connect(m_voiceBlock, &AACSpeechVoiceBlock::recheckRequested,
            this, &AACSpeechSettingsScreen::reloadVoices);
    connect(m_shapingBlock, &AACSpeechShapingBlock::shapingChanged,
            this, &AACSpeechSettingsScreen::onShapingChanged);
    connect(m_shapingBlock, &AACSpeechShapingBlock::previewRequested,
            this, &AACSpeechSettingsScreen::onPreviewRequested);
    connect(m_typingBlock, &AACSpeechTypingBlock::typingModeChanged,
            this, &AACSpeechSettingsScreen::onTypingModeChanged);
    connect(m_typingBlock, &AACSpeechTypingBlock::fatigueModeChanged,
            this, &AACSpeechSettingsScreen::onFatigueModeChanged);
    connect(m_phrasesBlock, &AACSpeechPhrasesBlock::previewPhraseRequested,
            this, &AACSpeechSettingsScreen::onPhrasePreviewRequested);
    connect(m_historyBlock, &AACSpeechHistoryRecoveryBlock::historyItemSelected,
            this, &AACSpeechSettingsScreen::onHistoryItemSelected);
    connect(m_historyBlock, &AACSpeechHistoryRecoveryBlock::lockChanged,
            this, &AACSpeechSettingsScreen::onLockChanged);
    connect(m_historyBlock, &AACSpeechHistoryRecoveryBlock::lockTimerChanged,
            this, &AACSpeechSettingsScreen::onLockTimerChanged);
    connect(m_historyBlock, &AACSpeechHistoryRecoveryBlock::recoveryRequested,
            this, &AACSpeechSettingsScreen::onRecoveryRequested);
    connect(m_exportBlock, &AACSpeechExportBlock::exportRequested,
            this, &AACSpeechSettingsScreen::onExportRequested);
    connect(m_exportBlock, &AACSpeechExportBlock::importRequested,
            this, &AACSpeechSettingsScreen::onImportRequested);
    connect(m_exportBlock, &AACSpeechExportBlock::snapshotRequested,
            this, &AACSpeechSettingsScreen::onSnapshotRequested);
    connect(m_exportBlock, &AACSpeechExportBlock::qrRequested,
            this, &AACSpeechSettingsScreen::onQrRequested);
    connect(m_resetBlock, &AACSpeechResetBlock::resetRequested,
            this, &AACSpeechSettingsScreen::onResetRequested);
connect(m_typingBlock, &AACSpeechTypingBlock::fatigueModeChanged,
        m_aac, &AACAccessibilityManager::setFatigueMode);
}

void AACSpeechSettingsScreen::reloadVoices()
{
    const QVector<QVoice> voices = m_tts->availableVoices();
    m_voiceResolver.setVoices(voices);
    m_hasVoices = m_voiceResolver.hasVoices();
    m_voiceBlock->setVoices(m_voiceResolver.voiceInfos());
    m_voiceBlock->setHasVoices(m_hasVoices);
    m_voiceBlock->setStabilityInfo(voices.size(),
                                   m_lastVoiceChange.isValid()
                                       ? m_lastVoiceChange.toString(Qt::ISODate)
                                       : tr("Unknown"));
}

void AACSpeechSettingsScreen::updateIdentityBlock()
{
    if (!m_hasVoices || m_lastVoiceIndex < 0
        || m_lastVoiceIndex >= m_voiceResolver.voiceInfos().size())
        return;

    const auto &info = m_voiceResolver.voiceInfos().at(m_lastVoiceIndex);
    const QLocale loc = info.voice.locale();

    m_identityBlock->setCurrentVoiceInfo(info.voice.name(),
                                         loc.name(),
                                         QStringLiteral("QTextToSpeech"),
                                         QStringLiteral("-"));

    m_identityBlock->setRegionalInfo(currentRegionalLocales());
}

void AACSpeechSettingsScreen::updateVoiceBlock()
{
    m_voiceBlock->setHasVoices(m_hasVoices);
}

void AACSpeechSettingsScreen::updateStatusBlock()
{
    const auto state = m_tts->state();
    const QString status = m_safety.statusForState(state, m_hasVoices);
    m_statusBlock->setStatusText(status);

    if (!m_hasVoices)
        m_statusBlock->setDetailText(m_safety.noVoicesDetail());
    else if (state == QTextToSpeech::Error)
        m_statusBlock->setDetailText(m_safety.engineErrorDetail());
    else
        m_statusBlock->setDetailText(QString());
}

void AACSpeechSettingsScreen::updateAnalyticsBlock()
{
    QString bestPreset;
    int bestPresetCount = 0;
    for (auto it = m_presetUsage.cbegin(); it != m_presetUsage.cend(); ++it) {
        if (it.value() > bestPresetCount) {
            bestPresetCount = it.value();
            bestPreset = it.key();
        }
    }

    QString bestVoice;
    int bestVoiceCount = 0;
    for (auto it = m_voiceUsage.cbegin(); it != m_voiceUsage.cend(); ++it) {
        if (it.value() > bestVoiceCount) {
            bestVoiceCount = it.value();
            bestVoice = it.key();
        }
    }

    int avgRate   = (m_shapingSamples > 0) ? m_rateSum   / m_shapingSamples : 0;
    int avgPitch  = (m_shapingSamples > 0) ? m_pitchSum  / m_shapingSamples : 0;
    int avgVolume = (m_shapingSamples > 0) ? m_volumeSum / m_shapingSamples : 0;

    m_analyticsBlock->setMostUsedPreset(bestPreset);
    m_analyticsBlock->setMostUsedVoice(bestVoice);
    m_analyticsBlock->setAverageRate(avgRate);
    m_analyticsBlock->setAveragePitch(avgPitch);
    m_analyticsBlock->setAverageVolume(avgVolume);
}

void AACSpeechSettingsScreen::updateHistoryBlock()
{
    m_historyBlock->setHistory(m_voiceHistory);
    m_historyBlock->setLockEnabled(m_lockEnabled);
    m_historyBlock->setLockTimerMinutes(m_lockMinutes);
}

// ------------------------------------------------------------
// Behaviour
// ------------------------------------------------------------

void AACSpeechSettingsScreen::applyPreset(int index)
{
    if (index < 0 || index >= m_presetEngine.presetCount())
        return;

    const auto &preset = m_presetEngine.presetAt(index);
    int voiceIndex = resolveVoiceIndexForPreset(preset);
    if (voiceIndex >= 0) {
        applyVoiceByIndex(voiceIndex);
        m_voiceBlock->setCurrentVoiceIndex(voiceIndex);
    }

    m_shapingBlock->setRate(preset.rate);
    m_shapingBlock->setPitch(preset.pitch);
    m_shapingBlock->setVolume(preset.volume);

    applyCurrentShapingToTts();
    recordVoiceUsage(QString::fromLatin1(preset.id),
                     voiceIndex >= 0 ? m_voiceResolver.voiceInfos().at(voiceIndex).voice.name()
                                     : QString());
}

void AACSpeechSettingsScreen::applyCurrentShapingToTts()
{
    if (!m_tts)
        return;

    const double rate   = AACPresetEngine::mapRateToTts(m_shapingBlock->rate());
    const double pitch  = AACPresetEngine::mapPitchToTts(m_shapingBlock->pitch());
    const double volume = AACPresetEngine::mapVolumeToTts(m_shapingBlock->volume());

    m_tts->setRate(rate);
    m_tts->setPitch(pitch);
    m_tts->setVolume(volume);

    recordShapingSample(m_shapingBlock->rate(),
                        m_shapingBlock->pitch(),
                        m_shapingBlock->volume());
}

void AACSpeechSettingsScreen::applyVoiceByIndex(int index)
{
    if (!m_tts || index < 0 || index >= m_voiceResolver.voiceInfos().size())
        return;

    const auto &info = m_voiceResolver.voiceInfos().at(index);
    m_tts->setVoice(info.voice);
    m_lastVoiceIndex = index;
    m_lastVoiceChange = QDateTime::currentDateTime();
    rememberCurrentVoiceIndex();
    updateIdentityBlock();
    updateVoiceBlock();
}

int AACSpeechSettingsScreen::resolveVoiceIndexForPreset(const AACPresetEngine::Preset &preset) const
{
    if (!preset.preferredVoiceName || !*preset.preferredVoiceName)
        return m_voiceResolver.fallbackIndex();
    return m_voiceResolver.findByNameFragment(QString::fromLatin1(preset.preferredVoiceName));
}

void AACSpeechSettingsScreen::rememberCurrentVoiceIndex()
{
    if (m_lastVoiceIndex < 0 || m_lastVoiceIndex >= m_voiceResolver.voiceInfos().size())
        return;

    const QString name = m_voiceResolver.voiceInfos().at(m_lastVoiceIndex).voice.name();
    if (!name.isEmpty() && !m_voiceHistory.contains(name)) {
        m_voiceHistory.prepend(name);
        while (m_voiceHistory.size() > 10)
            m_voiceHistory.removeLast();
    }
}

void AACSpeechSettingsScreen::loadSettings()
{
    QSettings s(settingsPathForProfile(m_profileId), QSettings::IniFormat);

    const int presetIndex = s.value("presetIndex", 0).toInt();
    m_presetsBlock->setCurrentPresetIndex(presetIndex);

    const int rate   = s.value("rate", 0).toInt();
    const int pitch  = s.value("pitch", 0).toInt();
    const int volume = s.value("volume", 80).toInt();
    m_shapingBlock->setRate(rate);
    m_shapingBlock->setPitch(pitch);
    m_shapingBlock->setVolume(volume);

    const int voiceIndex = s.value("voiceIndex", -1).toInt();
    if (voiceIndex >= 0) {
        m_voiceBlock->setCurrentVoiceIndex(voiceIndex);
        applyVoiceByIndex(voiceIndex);
    }

    const int mode = s.value("typingMode", int(SpeakAsYouTypeMode::Off)).toInt();
    const bool fatigue = s.value("fatigueMode", false).toBool();
    m_typingBlock->setMode(static_cast<SpeakAsYouTypeMode>(mode));
    m_typingBlock->setFatigueMode(fatigue);

    m_voiceHistory = s.value("voiceHistory").toStringList();
    m_lockEnabled  = s.value("lockEnabled", false).toBool();
    m_lockMinutes  = s.value("lockMinutes", 0).toInt();

    m_presetUsage = s.value("presetUsage").value<QMap<QString,int>>();
    m_voiceUsage  = s.value("voiceUsage").value<QMap<QString,int>>();
    m_rateSum     = s.value("rateSum", 0).toInt();
    m_pitchSum    = s.value("pitchSum", 0).toInt();
    m_volumeSum   = s.value("volumeSum", 0).toInt();
    m_shapingSamples = s.value("shapingSamples", 0).toInt();
}

void AACSpeechSettingsScreen::saveSettings() const
{
    QSettings s(settingsPathForProfile(m_profileId), QSettings::IniFormat);

    s.setValue("presetIndex", m_presetsBlock->currentPresetIndex());
    s.setValue("rate",   m_shapingBlock->rate());
    s.setValue("pitch",  m_shapingBlock->pitch());
    s.setValue("volume", m_shapingBlock->volume());
    s.setValue("voiceIndex", m_lastVoiceIndex);
    s.setValue("typingMode", int(m_typingBlock->mode()));
    s.setValue("fatigueMode", m_typingBlock->fatigueMode());
    s.setValue("voiceHistory", m_voiceHistory);
    s.setValue("lockEnabled", m_lockEnabled);
    s.setValue("lockMinutes", m_lockMinutes);

    s.setValue("presetUsage", QVariant::fromValue(m_presetUsage));
    s.setValue("voiceUsage",  QVariant::fromValue(m_voiceUsage));
    s.setValue("rateSum",     m_rateSum);
    s.setValue("pitchSum",    m_pitchSum);
    s.setValue("volumeSum",   m_volumeSum);
    s.setValue("shapingSamples", m_shapingSamples);
}

void AACSpeechSettingsScreen::recordVoiceUsage(const QString &presetId,
                                               const QString &voiceName)
{
    if (!presetId.isEmpty())
        m_presetUsage[presetId] += 1;
    if (!voiceName.isEmpty())
        m_voiceUsage[voiceName] += 1;
}

void AACSpeechSettingsScreen::recordShapingSample(int rate, int pitch, int volume)
{
    m_rateSum   += rate;
    m_pitchSum  += pitch;
    m_volumeSum += volume;
    m_shapingSamples += 1;
}

QStringList AACSpeechSettingsScreen::currentRegionalLocales() const
{
    QStringList out;
    if (m_lastVoiceIndex < 0 || m_lastVoiceIndex >= m_voiceResolver.voiceInfos().size())
        return out;

    const QLocale loc = m_voiceResolver.voiceInfos().at(m_lastVoiceIndex).voice.locale();
    out << loc.name();
    return out;
}

// ------------------------------------------------------------
// JSON export/import (sketch; adapt to your existing logic)
// ------------------------------------------------------------

QByteArray AACSpeechSettingsScreen::exportProfileJson() const
{
    QJsonObject root;
    root["presetIndex"] = m_presetsBlock->currentPresetIndex();
    root["rate"]   = m_shapingBlock->rate();
    root["pitch"]  = m_shapingBlock->pitch();
    root["volume"] = m_shapingBlock->volume();
    root["typingMode"] = int(m_typingBlock->mode());
    root["fatigueMode"] = m_typingBlock->fatigueMode();

    QJsonArray history;
    for (const QString &v : m_voiceHistory)
        history.append(v);
    root["voiceHistory"] = history;

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

bool AACSpeechSettingsScreen::importProfileJson(const QByteArray &data)
{
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return false;
    const QJsonObject root = doc.object();

    m_presetsBlock->setCurrentPresetIndex(root.value("presetIndex").toInt(0));
    m_shapingBlock->setRate(root.value("rate").toInt(0));
    m_shapingBlock->setPitch(root.value("pitch").toInt(0));
    m_shapingBlock->setVolume(root.value("volume").toInt(80));

    const int mode = root.value("typingMode").toInt(int(SpeakAsYouTypeMode::Off));
    const bool fatigue = root.value("fatigueMode").toBool(false);
    m_typingBlock->setMode(static_cast<SpeakAsYouTypeMode>(mode));
    m_typingBlock->setFatigueMode(fatigue);

    m_voiceHistory.clear();
    const QJsonArray history = root.value("voiceHistory").toArray();
    for (const QJsonValue &v : history)
        m_voiceHistory.append(v.toString());

    updateHistoryBlock();
    updateAnalyticsBlock();
    return true;
}

// ------------------------------------------------------------
// Slots
// ------------------------------------------------------------

void AACSpeechSettingsScreen::onPresetChanged(int index)
{
    applyPreset(index);
    updateAnalyticsBlock();
}

void AACSpeechSettingsScreen::onVoiceChanged(int index)
{
    applyVoiceByIndex(index);
}

void AACSpeechSettingsScreen::onShapingChanged()
{
    applyCurrentShapingToTts();
    updateAnalyticsBlock();
}

void AACSpeechSettingsScreen::onPreviewRequested(double rate, double pitch, double volume, int)
{
    if (!m_tts)
        return;
    m_tts->setRate(rate);
    m_tts->setPitch(pitch);
    m_tts->setVolume(volume);
    m_tts->say(tr("This is my voice."));
}

void AACSpeechSettingsScreen::onTypingModeChanged(SpeakAsYouTypeMode mode)
{
    emit speakAsYouTypeModeChanged(mode);
}

void AACSpeechSettingsScreen::onFatigueModeChanged(bool on)
{
    emit fatigueModeChanged(on);
}

void AACSpeechSettingsScreen::onPhrasePreviewRequested(const QString &text)
{
    if (!m_tts)
        return;
    m_tts->say(text);
}

void AACSpeechSettingsScreen::onHistoryItemSelected(int index)
{
    if (index < 0 || index >= m_voiceHistory.size())
        return;
    const QString name = m_voiceHistory.at(index);
    const int voiceIndex = m_voiceResolver.findByNameFragment(name);
    if (voiceIndex >= 0) {
        m_voiceBlock->setCurrentVoiceIndex(voiceIndex);
        applyVoiceByIndex(voiceIndex);
    }
}

void AACSpeechSettingsScreen::onLockChanged(bool locked)
{
    m_lockEnabled = locked;
    m_voiceBlock->setLocked(locked);
}

void AACSpeechSettingsScreen::onLockTimerChanged(int minutes)
{
    m_lockMinutes = minutes;
}

void AACSpeechSettingsScreen::onRecoveryRequested()
{
    if (!m_voiceHistory.isEmpty()) {
        const QString name = m_voiceHistory.first();
        const int voiceIndex = m_voiceResolver.findByNameFragment(name);
        if (voiceIndex >= 0) {
            m_voiceBlock->setCurrentVoiceIndex(voiceIndex);
            applyVoiceByIndex(voiceIndex);
        }
    }
}

void AACSpeechSettingsScreen::onExportRequested()
{
    const QByteArray json = exportProfileJson();
    QFile f(exportPathForProfile(m_profileId));
    if (f.open(QIODevice::WriteOnly))
        f.write(json);
}

void AACSpeechSettingsScreen::onImportRequested()
{
    QFile f(exportPathForProfile(m_profileId));
    if (!f.open(QIODevice::ReadOnly))
        return;
    const QByteArray data = f.readAll();
    if (importProfileJson(data)) {
        applyPreset(m_presetsBlock->currentPresetIndex());
        applyCurrentShapingToTts();
    }
}

void AACSpeechSettingsScreen::onSnapshotRequested()
{
    const QByteArray json = exportProfileJson();
    QGuiApplication::clipboard()->setText(QString::fromUtf8(json));
}

void AACSpeechSettingsScreen::onQrRequested()
{
    const QByteArray json = exportProfileJson();
    QGuiApplication::clipboard()->setText(QString::fromUtf8(json));
}

void AACSpeechSettingsScreen::onResetRequested()
{
    m_presetsBlock->setCurrentPresetIndex(0);
    m_shapingBlock->setRate(0);
    m_shapingBlock->setPitch(0);
    m_shapingBlock->setVolume(80);
    m_typingBlock->setMode(SpeakAsYouTypeMode::Off);
    m_typingBlock->setFatigueMode(false);
    m_voiceHistory.clear();
    m_lockEnabled = false;
    m_lockMinutes = 0;
    m_presetUsage.clear();
    m_voiceUsage.clear();
    m_rateSum = m_pitchSum = m_volumeSum = m_shapingSamples = 0;

    updateHistoryBlock();
    updateAnalyticsBlock();
    applyCurrentShapingToTts();
}

void AACSpeechSettingsScreen::onTtsStateChanged(QTextToSpeech::State)
{
    updateStatusBlock();
}
