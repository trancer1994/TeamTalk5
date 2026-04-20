#pragma once

#include "AACScreenBase.h"
#include <QVector>
#include <QVoice>
#include <QTextToSpeech>
#include <QLocale>
#include <QMap>
#include <QDateTime>

class QComboBox;
class QSlider;
class QLabel;
class QTableWidget;
class QLineEdit;
class QTextEdit;

class AACButton;
class AACToggle;

/*
 * Speak-as-you-type behaviour
 */
enum class SpeakAsYouTypeMode {
    Off = 0,
    Word,
    Sentence
};

/*
 * AACVoiceResolver / AACPresetEngine / AACSpeechSafety
 * (unchanged from your original header)
 */
class AACVoiceResolver
{
public:
    struct VoiceInfo {
        QVoice  voice;
        QString label;
    };

    void setVoices(const QVector<QVoice> &voices);
    const QVector<VoiceInfo> &voiceInfos() const { return m_voiceInfos; }
    bool hasVoices() const { return !m_voiceInfos.isEmpty(); }

    int findByNameFragment(const QString &fragment) const;
    int findClosestByLocaleHint(QLocale::Language lang,
                                QLocale::Country country = QLocale::AnyCountry) const;

    int fallbackIndex() const { return m_voiceInfos.isEmpty() ? -1 : 0; }

private:
    QVector<VoiceInfo> m_voiceInfos;
};

class AACPresetEngine
{
public:
    struct Preset {
        const char *id;
        const char *label;
        const char *preferredVoiceName;
        int rate;
        int pitch;
        int volume;
        const char *category;
        const char *description;
    };

    AACPresetEngine();

    int presetCount() const { return m_presetCount; }
    const Preset &presetAt(int index) const { return m_presets[index]; }
    int indexForId(const QString &id) const;

    static double mapRateToTts(int rate);
    static double mapPitchToTts(int pitch);
    static double mapVolumeToTts(int volume);

private:
    const Preset *m_presets = nullptr;
    int m_presetCount = 0;
};

class AACSpeechSafety
{
public:
    QString statusForState(QTextToSpeech::State state,
                           bool hasVoices) const;

    QString noVoicesDetail() const;
    QString engineErrorDetail() const;
};

/*
 * AAC‑native flat blocks
 * (all QGroupBox removed, all checkboxes → AACToggle, all buttons → AACButton)
 */

class AACSpeechIdentityBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechIdentityBlock(QWidget *parent = nullptr);

    void setCurrentVoiceInfo(const QString &name,
                             const QString &locale,
                             const QString &engine,
                             const QString &category);
    void setRegionalInfo(const QStringList &locales);
    void setIdentityNotes(const QString &notes);
    QString identityNotes() const;

private:
    QLabel    *m_voiceNameLabel = nullptr;
    QLabel    *m_localeLabel = nullptr;
    QLabel    *m_engineLabel = nullptr;
    QLabel    *m_categoryLabel = nullptr;
    QLabel    *m_regionalLabel = nullptr;
    QTextEdit *m_notesEdit = nullptr;
};

class AACSpeechPresetsBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechPresetsBlock(QWidget *parent = nullptr);

    void populatePresets(const AACPresetEngine &engine);
    void setCurrentPresetIndex(int index);
    int currentPresetIndex() const;

signals:
    void presetChanged(int index);

private:
    QComboBox *m_presetCombo = nullptr;
    QLabel    *m_presetDescription = nullptr;

private slots:
    void onPresetComboChanged(int index);
};

class AACSpeechVoiceBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechVoiceBlock(QWidget *parent = nullptr);

    void setVoices(const QVector<AACVoiceResolver::VoiceInfo> &infos);
    void setHasVoices(bool has);
    void setCurrentVoiceIndex(int index);
    int currentVoiceIndex() const;

    void setStabilityInfo(int voiceCount, const QString &lastChange);
    void setSafeMode(bool safeMode, const QString &detail);

    void setLocked(bool locked);

signals:
    void voiceChanged(int index);
    void recheckRequested();

private:
    QComboBox  *m_voiceCombo = nullptr;
    QLabel     *m_stabilityLabel = nullptr;
    QLabel     *m_safeModeLabel = nullptr;
    AACButton  *m_recheckButton = nullptr;
    bool        m_locked = false;

private slots:
    void onVoiceComboChanged(int index);
    void onRecheckClicked();
};

class AACSpeechShapingBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechShapingBlock(QWidget *parent = nullptr);

    void setRate(int v);
    void setPitch(int v);
    void setVolume(int v);

    int rate() const;
    int pitch() const;
    int volume() const;

signals:
    void shapingChanged();
    void previewRequested(double rate, double pitch, double volume, int speedPreset);

private:
    QSlider *m_rateSlider = nullptr;
    QSlider *m_pitchSlider = nullptr;
    QSlider *m_volumeSlider = nullptr;

    AACButton *m_previewButton = nullptr;
    AACButton *m_previewSlowButton = nullptr;
    AACButton *m_previewMediumButton = nullptr;
    AACButton *m_previewFastButton = nullptr;

private slots:
    void onSliderChanged();
    void onPreviewClicked();
    void onPreviewSpeedClicked();
};

class AACSpeechTypingBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechTypingBlock(QWidget *parent = nullptr);

    void setMode(SpeakAsYouTypeMode mode);
    SpeakAsYouTypeMode mode() const;

    void setFatigueMode(bool on);
    bool fatigueMode() const;

signals:
    void typingModeChanged(SpeakAsYouTypeMode mode);
    void fatigueModeChanged(bool on);

private:
    QComboBox *m_modeCombo = nullptr;
    AACToggle *m_fatigueToggle = nullptr;

private slots:
    void onModeChanged(int index);
    void onFatigueChanged(bool on);
};

class AACSpeechPronunciationBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechPronunciationBlock(QWidget *parent = nullptr);

    void setEntries(const QList<QPair<QString, QString>> &entries);
    QList<QPair<QString, QString>> entries() const;

private:
    QTableWidget *m_table = nullptr;
};

class AACSpeechPhrasesBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechPhrasesBlock(QWidget *parent = nullptr);

signals:
    void previewPhraseRequested(const QString &text);

private slots:
    void onIdentityPhraseClicked();
    void onAssistPhraseClicked();
    void onPleaseWaitPhraseClicked();
    void onEmergencyHelpClicked();
    void onEmergencyDangerClicked();

private:
    AACButton *m_identityBtn = nullptr;
    AACButton *m_assistBtn = nullptr;
    AACButton *m_waitBtn = nullptr;
    AACButton *m_emergencyHelpBtn = nullptr;
    AACButton *m_emergencyDangerBtn = nullptr;
};

class AACSpeechHistoryRecoveryBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechHistoryRecoveryBlock(QWidget *parent = nullptr);

    void setHistory(const QStringList &items);
    void setLockEnabled(bool locked);
    bool lockEnabled() const;
    void setLockTimerMinutes(int minutes);
    int lockTimerMinutes() const;

signals:
    void historyItemSelected(int index);
    void lockChanged(bool locked);
    void lockTimerChanged(int minutes);
    void recoveryRequested();

private:
    QComboBox  *m_historyCombo = nullptr;
    AACToggle  *m_lockToggle = nullptr;
    QComboBox  *m_lockTimerCombo = nullptr;
    AACButton  *m_recoverButton = nullptr;

private slots:
    void onHistoryChanged(int index);
    void onLockChanged(bool on);
    void onLockTimerChanged(int index);
    void onRecoverClicked();
};

class AACSpeechExportBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechExportBlock(QWidget *parent = nullptr);

signals:
    void exportRequested();
    void importRequested();
    void snapshotRequested();
    void qrRequested();

private slots:
    void onExportClicked();
    void onImportClicked();
    void onSnapshotClicked();
    void onQrClicked();

private:
    AACButton *m_exportButton = nullptr;
    AACButton *m_importButton = nullptr;
    AACButton *m_snapshotButton = nullptr;
    AACButton *m_qrButton = nullptr;
};

class AACSpeechAnalyticsBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechAnalyticsBlock(QWidget *parent = nullptr);

    void setMostUsedPreset(const QString &id);
    void setMostUsedVoice(const QString &name);
    void setAverageRate(int v);
    void setAveragePitch(int v);
    void setAverageVolume(int v);

private:
    QLabel *m_presetLabel = nullptr;
    QLabel *m_voiceLabel = nullptr;
    QLabel *m_rateLabel = nullptr;
    QLabel *m_pitchLabel = nullptr;
    QLabel *m_volumeLabel = nullptr;
};

class AACSpeechResetBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechResetBlock(QWidget *parent = nullptr);

signals:
    void resetRequested();

private slots:
    void onResetClicked();

private:
    AACButton *m_resetButton = nullptr;
};

class AACSpeechStatusBlock : public QWidget
{
    Q_OBJECT
public:
    explicit AACSpeechStatusBlock(QWidget *parent = nullptr);

    void setStatusText(const QString &text);
    void setDetailText(const QString &text);

private:
    QLabel *m_statusLabel = nullptr;
    QLabel *m_detailLabel = nullptr;
};

/*
 * AACSpeechSettingsScreen — AACScreenBase version
 */
class AACSpeechSettingsScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit AACSpeechSettingsScreen(QWidget *parent = nullptr);
    ~AACSpeechSettingsScreen() override;

    void setProfileId(const QString &profileId);

    SpeakAsYouTypeMode speakAsYouTypeMode() const;
    bool fatigueMode() const;

signals:
    void speakAsYouTypeModeChanged(SpeakAsYouTypeMode mode);
    void fatigueModeChanged(bool on);

private slots:
    void onPresetChanged(int index);
    void onVoiceChanged(int index);
    void onShapingChanged();
    void onPreviewRequested(double rate, double pitch, double volume, int speedPreset);
    void onTypingModeChanged(SpeakAsYouTypeMode mode);
    void onFatigueModeChanged(bool on);
    void onPhrasePreviewRequested(const QString &text);
    void onHistoryItemSelected(int index);
    void onLockChanged(bool locked);
    void onLockTimerChanged(int minutes);
    void onRecoveryRequested();
    void onExportRequested();
    void onImportRequested();
    void onSnapshotRequested();
    void onQrRequested();
    void onResetRequested();
    void onTtsStateChanged(QTextToSpeech::State state);

private:
    void setupUi();
    void initTts();
    void connectSignals();

    void reloadVoices();
    void updateIdentityBlock();
    void updateVoiceBlock();
    void updateStatusBlock();
    void updateAnalyticsBlock();
    void updateHistoryBlock();

    void applyPreset(int index);
    void applyCurrentShapingToTts();
    void applyVoiceByIndex(int index);
    int  resolveVoiceIndexForPreset(const AACPresetEngine::Preset &preset) const;
    void rememberCurrentVoiceIndex();

    void loadSettings();
    void saveSettings() const;

    void recordVoiceUsage(const QString &presetId, const QString &voiceName);
    void recordShapingSample(int rate, int pitch, int volume);

    QStringList currentRegionalLocales() const;

    QByteArray exportProfileJson() const;
    bool importProfileJson(const QByteArray &data);

    // AACScreenAdapter overrides
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;

private:
    QVBoxLayout *m_rootLayout = nullptr;

    QTextToSpeech *m_tts = nullptr;

    AACVoiceResolver m_voiceResolver;
    AACPresetEngine  m_presetEngine;
    AACSpeechSafety  m_safety;

    bool        m_hasVoices = false;
    int         m_lastVoiceIndex = -1;
    QString     m_profileId = QStringLiteral("default");
    QDateTime   m_lastVoiceChange;

    QMap<QString, int> m_presetUsage;
    QMap<QString, int> m_voiceUsage;
    int m_rateSum = 0;
    int m_pitchSum = 0;
    int m_volumeSum = 0;
    int m_shapingSamples = 0;

    QStringList m_voiceHistory;
    bool        m_lockEnabled = false;
    int         m_lockMinutes = 0;
    QDateTime   m_lockUntil;

    // Blocks
    AACSpeechIdentityBlock        *m_identityBlock = nullptr;
    AACSpeechPresetsBlock         *m_presetsBlock = nullptr;
    AACSpeechVoiceBlock           *m_voiceBlock = nullptr;
    AACSpeechShapingBlock         *m_shapingBlock = nullptr;
    AACSpeechTypingBlock          *m_typingBlock = nullptr;
    AACSpeechPronunciationBlock   *m_pronunciationBlock = nullptr;
    AACSpeechPhrasesBlock         *m_phrasesBlock = nullptr;
    AACSpeechHistoryRecoveryBlock *m_historyBlock = nullptr;
    AACSpeechExportBlock          *m_exportBlock = nullptr;
    AACSpeechAnalyticsBlock       *m_analyticsBlock = nullptr;
    AACSpeechResetBlock           *m_resetBlock = nullptr;
    AACSpeechStatusBlock          *m_statusBlock = nullptr;

    QVector<QWidget*> m_interactive;
};
