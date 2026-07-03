#pragma once

#include <QVector>
#include <QWidget>
#include <QtGlobal>
#include <QObject>
#include <QPointer>
#include <QList>
#include <QMap>
#include <QElapsedTimer>
#include <QRect>
#include <QSoundEffect>
#include <QTimer>
#include <QStringList>
#include <QQueue>

#include <memory>

#include "storage/aacstorage.h"
#include "core/AACProfile.h"
#include "core/AACProfileConfigTable.h"

enum class HapticSemantic {
    Background,   // scan ticks, low-importance pulses
    Confirm,      // focus, dwell complete
    Error         // errors, disconnects
};
enum class SpeechPriority {
    Critical,   // errors, disconnects
    System,     // connect, reconnect, join/leave
    User,       // user text, predictions
    Ambient     // hints, onboarding
};

struct SpeechItem {
    QString text;
    SpeechPriority priority;
};
class QWidget;
class QLayout;
class QEnterEvent;
class QFocusEvent;
class QMouseEvent;
class QPaintEvent;
class QTextToSpeech;

class AACLayoutEngine;
class AACFeedbackEngine;
class AACKeyboardScreen;
class AACSpeechEngine;
class AACMessageHistory;
class AACVocabularyManager;
class AACPredictionEngine;
class AACScreenAdapter;

class AACScreenAdapter {
public:
    virtual ~AACScreenAdapter() = default;

    virtual QList<QWidget*> interactiveWidgets() const = 0;
    virtual QList<QWidget*> primaryWidgets() const = 0;
    virtual QLayout* rootLayout() const = 0;
    virtual QWidget* predictiveStripContainer() const = 0;
};
struct AACModeFlags
{
    // Core interaction modes
    bool largeTargets      = false;
    bool dwell             = false;
    bool scanning          = false;
    bool stepScanning = false;
    bool reduceScanningSoundIntensity = false;
    bool auditoryFeedback  = false;
    bool hapticFeedback    = false;
    bool deepWells         = false;
bool feedbackEnabled = true;
    bool oneHandLayout     = false;
    bool ultraMinimal      = false;
    bool predictiveStrip   = false;
    bool highContrast      = false;

    // AAC-native feedback modes
    bool visualPulses      = true;   // visual confirmation pulses
    bool talkBackMode      = false;  // Android TalkBack detected
bool voiceOverMode = false;
bool switchControlScanning = false;
    bool fatigueMode       = false;  // softer/shorter feedback
    bool globalFeedback    = false;  // whole-screen pulses instead of key-only

    // Keyboard / symbol helpers
    bool coreSymbolsFirst  = false;
    bool curatedStripDwell = false;
bool autoReconnect = false;
bool helpMode = false;

    // Timing
    int dwellTimeMs        = 800;
    int scanningSpeedMs    = 1100;
    int touchHoldDelayMs   = 0;
};
struct UserProfile {
    QString nickname;
    int theme = 0;
    int language = 0;
};
class AACAccessibilityManager : public QObject
{
    Q_OBJECT

public:
    AACAccessibilityManager(QObject* parent = nullptr);

    // Singleton-style accessor for JNI bridge
    static AACAccessibilityManager* instance() { return s_instance; }

    void loadProfile();
    void saveProfile();

    // per‑user volume API (AAC‑side)
    int userVolume(const QString& userId) const;
    void setUserVolume(const QString& userId, int volume);
    QMap<QString,int> allUserVolumes() const { return m_userVolumes; }

    QStringList categories() const;
    QVector<AACVocabItem> words(const QString& category) const;

    void hydrate();
    void persist();
    void attachToAppLifecycle(QObject* app);
    void setKeyboardScanningLayout(const QVector<QVector<QWidget*>>& layout);

QString activeCategory() const          { return m_activeCategory; }
AACProfile profile() const              { return m_profile; }
AACModeFlags modes() const              { return m_modes; }
AACSpeechConfig speechConfig() const    { return m_speechConfig; }
bool predictionEnabled() const          { return m_predictionEnabled; }
UserProfile& userProfile()              { return m_userProfile; }
const UserProfile& userProfile() const  { return m_userProfile; }
QString lastHost() const                { return m_lastHost; }
quint16 lastPort() const                { return m_lastPort; }
QString lastUsername() const            { return m_lastUsername; }
QString lastPassword() const            { return m_lastPassword; }

bool voiceOverDetected() const          { return m_voiceOverDetected; }
bool switchControlDetected() const      { return m_switchControlDetected; }

    void setLastHost(const QString& h)      { m_lastHost = h; }
    void setLastPort(quint16 p)            { m_lastPort = p; }
    void setLastUsername(const QString& u) { m_lastUsername = u; }
    void setLastPassword(const QString& p) { m_lastPassword = p; }
    bool largeTargetsEnabled() const;
    bool highContrastEnabled() const;
    bool dwellEnabled() const;

    AACFeedbackEngine* feedbackEngine() const;
    AACSpeechEngine* speechEngine() const;
    AACMessageHistory* history() const;
    AACPredictionEngine* predictionEngine() const;
    AACVocabularyManager* vocabularyManager() const;

    // Live TalkBack update from Android (Java → C++)
    void updateTalkBackFromPlatform(bool on);

public slots:
    void setActiveCategory(const QString& category);
    void setProfile(AACProfile profile);
    void setModes(const AACModeFlags& modes);
    void setSpeechConfig(const AACSpeechConfig& config);
    void setPredictionEnabled(bool enabled);

    void boostPredictionVocabulary();
    void loadPredictionForUser(const QString& userId);
    void savePredictionForUser(const QString& userId);

    void setFatigueMode(bool on);
    void onVisualPulseRequested(int strength);

signals:
    void activeCategoryChanged(const QString& category);
    void profileChanged(AACProfile profile);
    void modesChanged(const AACModeFlags& modes);
    void speechConfigChanged(const AACSpeechConfig& config);
    void predictionEnabledChanged(bool enabled);
    void speechStarted(const QString& text);
    void speechFinished(const QString& text);
    void historyChanged(const QStringList& history);
    void largeTargetsChanged(bool on);
    void highContrastChanged(bool on);
    void dwellChanged(bool on);
    void keyboardScanningLayoutChanged(const QVector<QVector<QWidget*>>& layout);

private:
    QMap<QString,int> m_userVolumes;
    QString m_activeCategory;
    AACProfile m_profile = AACProfile::CoreVocabulary;
    AACModeFlags m_modes;
UserProfile m_userProfile;

    QString m_lastHost;
    quint16 m_lastPort = 0;
    QString m_lastUsername;
    QString m_lastPassword;

    AACLayoutEngine* m_layoutEngine = nullptr;
    AACFeedbackEngine* m_feedbackEngine = nullptr;

    AACSpeechEngine* m_speechEngine = nullptr;
    AACMessageHistory* m_history = nullptr;
    AACVocabularyManager* m_vocabularyManager = nullptr;

    AACPredictionEngine* m_predictionEngine = nullptr;
    bool m_predictionEnabled = true;

    AACSpeechConfig m_speechConfig;

    QVector<QVector<QWidget*>> m_keyboardScanningLayout;

    std::unique_ptr<AACStorage> m_storage;

    // Live TalkBack + other screenreader state
    bool m_talkBackDetected = false;
bool m_voiceOverDetected = false;
bool m_switchControlDetected = false;

    // Singleton pointer for JNI access
    static AACAccessibilityManager* s_instance;
};


class AACLayoutEngine : public QObject
{
    Q_OBJECT

public:
    AACLayoutEngine(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void applyLayout(AACScreenAdapter* screen);
    void applyLargeTargets(AACScreenAdapter* screen);
    void applyOneHandLayout(AACScreenAdapter* screen);
    void applyUltraMinimal(AACScreenAdapter* screen);
    void applyPredictiveStrip(AACScreenAdapter* screen);

private:
    void scaleInteractiveWidget(QWidget* w, bool enabled);
    void scaleLayout(QLayout* lay, bool enabled);

    AACAccessibilityManager* m_mgr = nullptr;
};

class AACFeedbackEngine : public QObject
{
    Q_OBJECT

public:
    AACFeedbackEngine(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void playClick();
    void playFocus();
    void playError();
    void playDwellComplete();
void playStepAdvance();
    void playIncomingMessage();
    void playPrivateMessage();
void playUserJoin();
void playUserLeave();
void playTransmitOn();
void playTransmitOff();


void playScreenChange();
void playMessageSent();
void playAdjust();
void playPredictiveUpdate();
void playModeChange();
void playPanelToggle();
void playConnectionLost();
void playConnectionRestored();
void playServerError();

    void hapticSoft();
    void hapticStrong();
    void hapticError();

signals:
    void visualPulseRequested(int strength);

private:
    void initSounds();
    void playEffectWithFatigue(QSoundEffect& eff,
                               float normalVol = 0.9f,
                               float fatigueVol = 0.55f);
    void doHaptic(int strength, HapticSemantic semantic = HapticSemantic::Confirm);
    bool isScanningSound(const QSoundEffect& eff) const;
    AACAccessibilityManager* m_mgr = nullptr;

    QSoundEffect m_clickSound;
    QSoundEffect m_focusSound;
    QSoundEffect m_errorSound;
    QSoundEffect m_dwellSound;
QSoundEffect m_incomingMessageSound;
QSoundEffect m_privateMessageSound;
QSoundEffect m_transmitOnSound;
QSoundEffect m_transmitOffSound;
QSoundEffect m_userJoinSound;
QSoundEffect m_userLeaveSound;
QSoundEffect m_screenChangeSound;
QSoundEffect m_messageSentSound;
QSoundEffect m_adjustSound;
QSoundEffect m_predictiveUpdateSound;
QSoundEffect m_modeChangeSound;
QSoundEffect m_panelToggleSound;
QSoundEffect m_connectionLostSound;
QSoundEffect m_connectionRestoredSound;
QSoundEffect m_serverErrorSound;
QSoundEffect m_backspaceSound;
QSoundEffect m_enterSound;
QSoundEffect m_actionSound;
};

class AACSpeechEngine : public QObject
{
    Q_OBJECT

public:
    AACSpeechEngine(AACAccessibilityManager* mgr, QObject* parent = nullptr);

bool shouldSpeakScanning() const;
    void speak(const QString& text);
void speakNotification(const QString& text);
void speakNotification(const QString& text, SpeechPriority p);
    void speakScanningRow(const QString& label);
    void speakScanningItem(const QString& label);
    void stop();

    void setVoice(const QString& voiceName);
    void setRate(double rate);
    void setPitch(double pitch);

    void speakLetter(const QString& letter);
    void speakWord(const QString& word);
    void speakPhrase(const QString& phrase);
    void echoOnSend(const QString& text);

public slots:
    void applyConfig(const AACSpeechConfig& cfg);

signals:
    void speechStarted(const QString& text);
    void speechFinished(const QString& text);

private:
    void intelligibilityShaping(AACSpeechConfig& cfg);
    void lowIntensityShaping(AACSpeechConfig& cfg);
    void applyPresetShaping(AACSpeechConfig& cfg);
    void speakQueued();

    AACAccessibilityManager* m_mgr = nullptr;
    QTextToSpeech* m_tts = nullptr;
    AACSpeechConfig m_cfg;
    AACSpeechConfig::SpeakAsYouTypeMode m_sayMode = AACSpeechConfig::SpeakNone;
QQueue<SpeechItem> m_queue;
bool m_speaking = false;
SpeechPriority m_currentPriority = SpeechPriority::Ambient;
};

class AACMessageHistory : public QObject
{
    Q_OBJECT

public:
    AACMessageHistory(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void addMessage(const QString& msg);
    QStringList history() const;

public slots:
    void replayMessage(int index);
    void replayLast();

signals:
    void historyChanged(const QStringList& history);

private:
    AACAccessibilityManager* m_mgr = nullptr;
    QStringList m_history;
};
class AACFramework : public QObject
{
    Q_OBJECT
public:
    explicit AACFramework(QObject* parent = nullptr);

    //
    // --- Core accessors used throughout your AAC stack ---
    //

    // Global AAC manager (modes, profile, prediction, speech, feedback)
    AACAccessibilityManager* accessibilityManager() const { return m_accessibility; }

    // Main keyboard screen
    AACKeyboardScreen* keyboardScreen() const { return m_keyboardScreen; }

    // Earcons / feedback engine
    AACFeedbackEngine* earcons() const
    {
        return m_accessibility ? m_accessibility->feedbackEngine() : nullptr;
    }

    // Speech engine (TTS)
    AACSpeechEngine* speechEngine() const
    {
        return m_accessibility ? m_accessibility->speechEngine() : nullptr;
    }

    // Prediction engine
    AACPredictionEngine* predictionEngine() const
    {
        return m_accessibility ? m_accessibility->predictionEngine() : nullptr;
    }

    // Message history
    AACMessageHistory* history() const
    {
        return m_accessibility ? m_accessibility->history() : nullptr;
    }

    // Convenience: expose AAC modes
    AACModeFlags modes() const
    {
        return m_accessibility ? m_accessibility->modes() : AACModeFlags{};
    }

    // Convenience: expose AAC profile
    AACProfile profile() const
    {
        return m_accessibility ? m_accessibility->profile() : AACProfile::CoreVocabulary;
    }

    //
    // Unified earcon routing entry point
    //
    void feedback(AACEvent ev);

private:
    AACAccessibilityManager* m_accessibility = nullptr;
    AACKeyboardScreen*       m_keyboardScreen = nullptr;
};
