#include "AACFramework.h"

#include <QTextToSpeech>
#include <QtGlobal>
#include <QPainter>
#include <QPen>
#include <QEnterEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QLayout>
#include <QAbstractButton>
#include <QFont>
#include <QGuiApplication>
#include <QScreen>
#include <QSet>
#include <QBuffer>
#include <QUrl>
#include <QMap>
#include <QtMath>

#include "AACVocabularyManager.h"
#if defined(Q_OS_IOS)
#include "haptics_ios.mm"
#elif defined(Q_OS_MACOS)
#include "haptics_macos.mm"
#elif defined(Q_OS_ANDROID)
#include "haptics_android.cpp"
#elif defined(Q_OS_WIN)
#include "haptics_windows.cpp"
#elif defined(Q_OS_LINUX)
#include "haptics_linux.cpp"
#endif

AACAccessibilityManager* AACAccessibilityManager::s_instance = nullptr;

AACAccessibilityManager::AACAccessibilityManager(QObject* parent)
    : QObject(parent)
{
    s_instance = this;

    m_storage = std::make_unique<AACStorage>();

    m_layoutEngine      = new AACLayoutEngine(this, this);
    m_feedbackEngine    = new AACFeedbackEngine(this, this);
    m_speechEngine      = new AACSpeechEngine(this, this);
    m_history           = new AACMessageHistory(this, this);
    m_vocabularyManager = new AACVocabularyManager(this);
    m_vocabularyManager->initialize();

    m_predictionEngine  = new AACPredictionEngine(this, this);

    connect(m_speechEngine, &AACSpeechEngine::speechStarted,
            this, &AACAccessibilityManager::speechStarted);
    connect(m_speechEngine, &AACSpeechEngine::speechFinished,
            this, &AACAccessibilityManager::speechFinished);

    connect(m_history, &AACMessageHistory::historyChanged,
            this, &AACAccessibilityManager::historyChanged);

    connect(this, &AACAccessibilityManager::speechConfigChanged,
            m_speechEngine, &AACSpeechEngine::applyConfig);

    connect(m_vocabularyManager, &AACVocabularyManager::vocabularyChanged,
            this, &AACAccessibilityManager::boostPredictionVocabulary);

    //
    // Only this — load profile + modes
    //
    loadProfile();
}

int AACAccessibilityManager::userVolume(const QString& userId) const
{
    auto it = m_userVolumes.constFind(userId);
    if (it == m_userVolumes.constEnd())
        return -1; // means "no stored volume"
    return it.value();
}

void AACAccessibilityManager::loadProfile()
{
    QSettings s;

    // Profile
    m_profile.configured   = s.value("aac/configured", false).toBool();
    m_profile.commMethod   = static_cast<CommMethod>(s.value("aac/commMethod", 0).toInt());
    m_profile.typingMethod = static_cast<TypingMethod>(s.value("aac/typingMethod", 0).toInt());
    m_profile.vocabId      = s.value("aac/vocabId", "core").toString();

    // Modes
    m_modes.largeTargets      = s.value("aac/modes/largeTargets", false).toBool();
    m_modes.dwell             = s.value("aac/modes/dwell", false).toBool();
    m_modes.scanning          = s.value("aac/modes/scanning", false).toBool();
    m_modes.auditoryFeedback  = s.value("aac/modes/auditoryFeedback", false).toBool();
    m_modes.hapticFeedback    = s.value("aac/modes/hapticFeedback", false).toBool();
    m_modes.deepWells         = s.value("aac/modes/deepWells", false).toBool();
    m_modes.oneHandLayout     = s.value("aac/modes/oneHandLayout", false).toBool();
    m_modes.ultraMinimal      = s.value("aac/modes/ultraMinimal", false).toBool();
    m_modes.predictiveStrip   = s.value("aac/modes/predictiveStrip", false).toBool();
    m_modes.highContrast      = s.value("aac/modes/highContrast", false).toBool();
    m_modes.fatigueMode       = s.value("aac/modes/fatigueMode", false).toBool();
    m_modes.coreSymbolsFirst  = s.value("aac/modes/coreSymbolsFirst", false).toBool();
    m_modes.curatedStripDwell = s.value("aac/modes/curatedStripDwell", false).toBool();

    // Newly added AAC-native feedback modes
    m_modes.visualPulses      = s.value("aac/modes/visualPulses", true).toBool();
    m_modes.talkBackMode      = s.value("aac/modes/talkBackMode", false).toBool();
    m_modes.globalFeedback    = s.value("aac/modes/globalFeedback", false).toBool();

    // Timing
    m_modes.dwellTimeMs       = s.value("aac/modes/dwellTimeMs", 800).toInt();
    m_modes.scanningSpeedMs   = s.value("aac/modes/scanningSpeedMs", 1100).toInt();
    m_modes.touchHoldDelayMs  = s.value("aac/modes/touchHoldDelayMs", 0).toInt();
m_userProfile.nickname = s.value("aac/profile/nickname", "").toString();
m_userProfile.theme    = s.value("aac/profile/theme", 0).toInt();
m_userProfile.language = s.value("aac/profile/language", 0).toInt();
    m_lastHost     = s.value("aac/connection/host", "").toString();
    m_lastPort     = s.value("aac/connection/port", 0).toUInt();
    m_lastUsername = s.value("aac/connection/username", "").toString();
    m_lastPassword = s.value("aac/connection/password", "").toString();
}

void AACAccessibilityManager::saveProfile()
{
    QSettings s;

    // Profile
    s.setValue("aac/configured",   m_profile.configured);
    s.setValue("aac/commMethod",   static_cast<int>(m_profile.commMethod));
    s.setValue("aac/typingMethod", static_cast<int>(m_profile.typingMethod));
    s.setValue("aac/vocabId",      m_profile.vocabId);

    // Modes
    s.setValue("aac/modes/largeTargets",      m_modes.largeTargets);
    s.setValue("aac/modes/dwell",             m_modes.dwell);
    s.setValue("aac/modes/scanning",          m_modes.scanning);
    s.setValue("aac/modes/auditoryFeedback",  m_modes.auditoryFeedback);
    s.setValue("aac/modes/hapticFeedback",    m_modes.hapticFeedback);
    s.setValue("aac/modes/deepWells",         m_modes.deepWells);
    s.setValue("aac/modes/oneHandLayout",     m_modes.oneHandLayout);
    s.setValue("aac/modes/ultraMinimal",      m_modes.ultraMinimal);
    s.setValue("aac/modes/predictiveStrip",   m_modes.predictiveStrip);
    s.setValue("aac/modes/highContrast",      m_modes.highContrast);
    s.setValue("aac/modes/fatigueMode",       m_modes.fatigueMode);
    s.setValue("aac/modes/coreSymbolsFirst",  m_modes.coreSymbolsFirst);
    s.setValue("aac/modes/curatedStripDwell", m_modes.curatedStripDwell);

    // Newly added AAC-native feedback modes
    s.setValue("aac/modes/visualPulses",      m_modes.visualPulses);
    s.setValue("aac/modes/talkBackMode",      m_modes.talkBackMode);
    s.setValue("aac/modes/globalFeedback",    m_modes.globalFeedback);

    // Timing
    s.setValue("aac/modes/dwellTimeMs",       m_modes.dwellTimeMs);
    s.setValue("aac/modes/scanningSpeedMs",   m_modes.scanningSpeedMs);
    s.setValue("aac/modes/touchHoldDelayMs",  m_modes.touchHoldDelayMs);
s.setValue("aac/profile/nickname", m_userProfile.nickname);
s.setValue("aac/profile/theme",    m_userProfile.theme);
s.setValue("aac/profile/language", m_userProfile.language);
    s.setValue("aac/connection/host",     m_lastHost);
    s.setValue("aac/connection/port",     m_lastPort);
    s.setValue("aac/connection/username", m_lastUsername);
    s.setValue("aac/connection/password", m_lastPassword);
}
void AACAccessibilityManager::setUserVolume(const QString& userId, int volume)
{
    // Clamp to TeamTalk’s valid range
    if (volume < soundsystem::VOLUME_MIN)
        volume = soundsystem::VOLUME_MIN;
    if (volume > soundsystem::VOLUME_MAX)
        volume = soundsystem::VOLUME_MAX;

    m_userVolumes[userId] = volume;
}
void AACAccessibilityManager::hydrate()
{
    if (m_storage)
        m_storage->hydrate(*this);
}

void AACAccessibilityManager::persist()
{
    if (m_storage)
        m_storage->persist(*this);
}

void AACAccessibilityManager::attachToAppLifecycle(QObject* app)
{
    if (!app)
        return;

    hydrate();

    connect(app, SIGNAL(aboutToQuit()),
            this, SLOT(persist()));

    // --- Android TalkBack detection ---
#if defined(Q_OS_ANDROID)
    connect(app, &QGuiApplication::applicationStateChanged,
            this, [this](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive) {
            QAndroidJniObject ctx = QtAndroid::androidContext();

            jboolean tb = QAndroidJniObject::callStaticMethod<jboolean>(
                "org/aacframework/HapticHelper",
                "isTalkBackEnabled",
                "(Landroid/content/Context;)Z",
                ctx.object()
            );

            m_talkBackDetected = tb;
            updateModesFromPlatform();
        }
    });
#endif

    // --- iOS VoiceOver + Switch Control detection ---
#if defined(Q_OS_IOS)
    // Initial detection
    m_voiceOverDetected     = UIAccessibilityIsVoiceOverRunning();
    m_switchControlDetected = UIAccessibilityIsSwitchControlRunning();
    updateModesFromPlatform();

    // Live updates
    [[NSNotificationCenter defaultCenter] addObserverForName:UIAccessibilityVoiceOverStatusDidChangeNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification*) {
        m_voiceOverDetected = UIAccessibilityIsVoiceOverRunning();
        updateModesFromPlatform();
    }];

    [[NSNotificationCenter defaultCenter] addObserverForName:UIAccessibilitySwitchControlStatusDidChangeNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification*) {
        m_switchControlDetected = UIAccessibilityIsSwitchControlRunning();
        updateModesFromPlatform();
    }];
#endif
}
void AACAccessibilityManager::updateModesFromPlatform()
{
    AACModeFlags modes = m_modes;

    modes.talkBackMode          = m_talkBackDetected;
    modes.voiceOverMode         = m_voiceOverDetected;
    modes.switchControlScanning = m_switchControlDetected;

    // Any screenreader forces visual pulses on
    if (m_talkBackDetected || m_voiceOverDetected || m_switchControlDetected)
        modes.visualPulses = true;

    setModes(modes);
}
void AACAccessibilityManager::setKeyboardScanningLayout(
        const QVector<QVector<QWidget*>>& layout)
{
    m_keyboardScanningLayout = layout;
    emit keyboardScanningLayoutChanged(m_keyboardScanningLayout);
}

QStringList AACAccessibilityManager::categories() const
{
    return m_vocabularyManager ? m_vocabularyManager->categories()
                               : QStringList();
}

QVector<AACVocabItem> AACAccessibilityManager::words(const QString& category) const
{
    return m_vocabularyManager ? m_vocabularyManager->wordsInCategory(category)
                               : QVector<AACVocabItem>();
}

bool AACAccessibilityManager::largeTargetsEnabled() const
{
    return m_modes.largeTargets;
}

bool AACAccessibilityManager::highContrastEnabled() const
{
    return m_modes.highContrast;
}

bool AACAccessibilityManager::dwellEnabled() const
{
    return m_modes.dwell;
}

AACFeedbackEngine* AACAccessibilityManager::feedbackEngine() const
{
    return m_feedbackEngine;
}

AACSpeechEngine* AACAccessibilityManager::speechEngine() const
{
    return m_speechEngine;
}

AACMessageHistory* AACAccessibilityManager::history() const
{
    return m_history;
}

AACPredictionEngine* AACAccessibilityManager::predictionEngine() const
{
    return m_predictionEngine;
}

AACVocabularyManager* AACAccessibilityManager::vocabularyManager() const
{
    return m_vocabularyManager;
}

void AACAccessibilityManager::setProfile(AACProfile profile)
{
    if (m_profile == profile)
        return;

    m_profile = profile;
    const AACProfileConfig cfg = profileConfig(profile);

    if (m_vocabularyManager)
        m_vocabularyManager->setSymbolPack(cfg.symbolPack);

    if (m_predictionEngine)
        m_predictionEngine->applyProfile(cfg);

    if (m_layoutEngine)
        m_layoutEngine->applyProfile(cfg);

AACModeFlags newModes = m_modes;

// ------------------------------------------------------------
// 1. Apply profile-specific overrides
// ------------------------------------------------------------
if (cfg.gridId == "scanning_1xn") {
    newModes.scanning       = true;
    newModes.dwell          = false;
    newModes.largeTargets   = true;
    newModes.oneHandLayout  = false;
    newModes.ultraMinimal   = true;
    newModes.predictiveStrip = false;
    newModes.dwellTimeMs    = 1100;
}
else if (cfg.gridId == "keyboard") {
    newModes.scanning       = false;
    newModes.dwell          = false;
    newModes.largeTargets   = false;
    newModes.oneHandLayout  = false;
    newModes.ultraMinimal   = false;
    newModes.predictiveStrip = true;
    newModes.dwellTimeMs    = 800;
}
else if (cfg.gridId == "sensory_2x2") {
    newModes.largeTargets   = true;
    newModes.ultraMinimal   = true;
    newModes.scanning       = false;
    newModes.dwell          = false;
    newModes.predictiveStrip = false;
    newModes.dwellTimeMs    = 900;
}
else {
    newModes.largeTargets   = true;
    newModes.ultraMinimal   = false;
    newModes.predictiveStrip = false;
    newModes.dwellTimeMs    = 1000;
}

// ------------------------------------------------------------
// 2. AAC-native automatic overrides
// ------------------------------------------------------------

// If dwell is enabled, enforce dwell timing
if (newModes.dwell) {
    newModes.dwellTimeMs = 900;   // AAC-standard default
}

// If scanning is enabled, enforce scanning speed
if (newModes.scanning) {
    newModes.scanningSpeedMs = 1100;  // AAC-standard default
}

// TalkBack mode (Android backend sets this)
if (m_talkBackDetected) {
    newModes.talkBackMode = true;
    newModes.visualPulses = true;      // always on for TalkBack
}

// Fatigue mode (user setting)
if (m_fatigueDetected) {
    newModes.fatigueMode = true;
}

// Global feedback mode (user setting)
if (m_globalFeedbackEnabled) {
    newModes.globalFeedback = true;
}

// ------------------------------------------------------------
// 3. Commit final modes
// ------------------------------------------------------------
setModes(newModes);

// ------------------------------------------------------------
// 4. Notify profile change
// ------------------------------------------------------------
emit profileChanged(profile);
}
void AACAccessibilityManager::setPredictionEnabled(bool enabled)
{
    if (m_predictionEnabled == enabled)
        return;

    m_predictionEnabled = enabled;
    emit predictionEnabledChanged(enabled);
}

void AACAccessibilityManager::setActiveCategory(const QString& category)
{
    if (m_activeCategory == category)
        return;

    m_activeCategory = category;
    emit activeCategoryChanged(category);

    if (m_predictionEngine)
        m_predictionEngine->setCurrentCategory(category);
}

void AACAccessibilityManager::boostPredictionVocabulary()
{
    if (!m_vocabularyManager || !m_predictionEngine)
        return;

    const QStringList words = m_vocabularyManager->allWords();
    for (const QString& w : words)
        m_predictionEngine->boostToken(w);
}

void AACAccessibilityManager::loadPredictionForUser(const QString& userId)
{
    if (!m_predictionEngine)
        return;

    const QString path = QStringLiteral("pred_%1.dat").arg(userId);
    m_predictionEngine->loadFromFile(path);
}

void AACAccessibilityManager::savePredictionForUser(const QString& userId)
{
    if (!m_predictionEngine)
        return;

    const QString path = QStringLiteral("pred_%1.dat").arg(userId);
    m_predictionEngine->saveToFile(path);
}

void AACAccessibilityManager::setModes(const AACModeFlags& modes)
{
    AACModeFlags old = m_modes;

    // If nothing changed, bail out early
    if (old.largeTargets      == modes.largeTargets      &&
        old.dwell             == modes.dwell             &&
        old.scanning          == modes.scanning          &&
        old.auditoryFeedback  == modes.auditoryFeedback  &&
        old.hapticFeedback    == modes.hapticFeedback    &&
        old.deepWells         == modes.deepWells         &&
        old.oneHandLayout     == modes.oneHandLayout     &&
        old.ultraMinimal      == modes.ultraMinimal      &&
        old.predictiveStrip   == modes.predictiveStrip   &&
        old.highContrast      == modes.highContrast      &&
        old.visualPulses      == modes.visualPulses      &&
        old.talkBackMode      == modes.talkBackMode      &&
        old.fatigueMode       == modes.fatigueMode       &&
        old.globalFeedback    == modes.globalFeedback    &&
        old.coreSymbolsFirst  == modes.coreSymbolsFirst  &&
        old.curatedStripDwell == modes.curatedStripDwell &&
        old.dwellTimeMs       == modes.dwellTimeMs       &&
        old.scanningSpeedMs   == modes.scanningSpeedMs   &&
        old.touchHoldDelayMs  == modes.touchHoldDelayMs)
    {
        return;
    }

    // Commit
    m_modes = modes;

    // Notify listeners
    emit modesChanged(m_modes);

    if (old.largeTargets != m_modes.largeTargets)
        emit largeTargetsChanged(m_modes.largeTargets);

    if (old.highContrast != m_modes.highContrast)
        emit highContrastChanged(m_modes.highContrast);

    if (old.dwell != m_modes.dwell)
        emit dwellChanged(m_modes.dwell);

    // Re-apply layout to the active screen
    if (m_layoutEngine) {
        if (auto* screen = m_layoutEngine->currentScreen()) {
            m_layoutEngine->applyLayout(screen);
        }
    }

    // Persist profile
    saveProfile();
}

void AACAccessibilityManager::setFatigueMode(bool on)
{
    m_modes.fatigueMode = on;
    emit modesChanged(m_modes);
    saveProfile();
}
void AACAccessibilityManager::setSpeechConfig(const AACSpeechConfig& cfg)
{
    m_speechConfig = cfg;
    emit speechConfigChanged(m_speechConfig);
}
void AACAccessibilityManager::onVisualPulseRequested(int strength)
{
    if (!m_layoutEngine)
        return;

    AACScreen* screen = m_layoutEngine->currentScreen();
    if (!screen)
        return;

    const AACModeFlags& modes = m_modes;

    // Visual pulses disabled
    if (!modes.visualPulses)
        return;

    // Global feedback → whole-screen pulse
    if (modes.globalFeedback) {
        screen->applyVisualPulse(strength);
        return;
    }

    // Try focused AACKeyButton first
    QWidget* focus = screen->focusWidget();
    if (auto* key = qobject_cast<AACKeyButton*>(focus)) {
        screen->applyVisualPulseTo(key, strength);
        return;
    }

    // Fallback: whole-screen pulse
    screen->applyVisualPulse(strength);
}
void AACAccessibilityManager::updateTalkBackFromPlatform(bool on)
{
    if (m_talkBackDetected == on)
        return;

    m_talkBackDetected = on;

    AACModeFlags modes = m_modes;
    modes.talkBackMode = on;

    if (on)
        modes.visualPulses = true;

    setModes(modes);
}
extern "C" JNIEXPORT void JNICALL
Java_org_aacframework_HapticHelper_nativeOnTalkBackChanged(
        JNIEnv* /*env*/, jclass /*clazz*/, jboolean enabled)
{
    if (auto* mgr = AACAccessibilityManager::instance()) {
        mgr->updateTalkBackFromPlatform(enabled);
    }
}
AACLayoutEngine::AACLayoutEngine(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
}

void AACLayoutEngine::applyLayout(AACScreenAdapter* screen)
{
    if (!screen)
        return;

    const AACModeFlags modes = m_mgr->modes();

    applyLargeTargets(screen);

    if (modes.oneHandLayout)
        applyOneHandLayout(screen);

    if (modes.ultraMinimal)
        applyUltraMinimal(screen);

    if (modes.predictiveStrip)
        applyPredictiveStrip(screen);
}

void AACLayoutEngine::applyLargeTargets(AACScreenAdapter* screen)
{
    const bool enabled = m_mgr->modes().largeTargets;

    for (QWidget* w : screen->interactiveWidgets())
        scaleInteractiveWidget(w, enabled);

    if (QLayout* lay = screen->rootLayout())
        scaleLayout(lay, enabled);
}

void AACLayoutEngine::applyOneHandLayout(AACScreenAdapter* screen)
{
    QLayout* lay = screen->rootLayout();
    if (!lay)
        return;

    QMargins m = lay->contentsMargins();
if (m_mgr->modes().oneHandLayout) {
        m.setBottom(m.bottom() + 40);
    } else {
        m.setBottom(m.bottom() + 40);
    }
    lay->setContentsMargins(m);
}

void AACLayoutEngine::applyUltraMinimal(AACScreenAdapter* screen)
{
    const bool enabled = m_mgr->modes().ultraMinimal;
    if (!enabled)
        return;

    const QList<QWidget*> primary = screen->primaryWidgets();
    const QSet<QWidget*> primarySet(primary.begin(), primary.end());

    for (QWidget* w : screen->interactiveWidgets()) {
        if (!primarySet.contains(w))
            w->setVisible(false);
    }
}

void AACLayoutEngine::applyPredictiveStrip(AACScreenAdapter* screen)
{
    Q_UNUSED(screen);
    // Placeholder for predictive strip widget placement.
}

void AACLayoutEngine::scaleInteractiveWidget(QWidget* w, bool enabled)
{
    if (!w)
        return;

    const bool isButton = qobject_cast<QAbstractButton*>(w) != nullptr;

    if (!isButton)
        return;

    if (enabled) {
        w->setMinimumSize(AAC_MIN_TARGET, AAC_MIN_TARGET);
        QFont f = w->font();
        f.setPointSizeF(f.pointSizeF() * AAC_FONT_SCALE);
        w->setFont(f);
    } else {
        w->setMinimumSize(0, 0);
        QFont f = w->font();
        f.setPointSizeF(f.pointSizeF() / AAC_FONT_SCALE);
        w->setFont(f);
    }
}

void AACLayoutEngine::scaleLayout(QLayout* lay, bool enabled)
{
    if (!lay)
        return;

    if (enabled) {
        lay->setSpacing(AAC_MIN_SPACING);
        lay->setContentsMargins(
            AAC_MIN_SPACING,
            AAC_MIN_SPACING,
            AAC_MIN_SPACING,
            AAC_MIN_SPACING
        );
    } else {
        lay->setSpacing(8);
        lay->setContentsMargins(8, 8, 8, 8);
    }
}

// =====================================================================
// AACFeedbackEngine — FULL REWRITE WITH FATIGUE + HAPTICS
// =====================================================================

AACFeedbackEngine::AACFeedbackEngine(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
    initSounds();

    connect(this, &AACFeedbackEngine::visualPulseRequested,
            m_mgr, &AACAccessibilityManager::onVisualPulseRequested);
}

// -----------------------------
// Tone generation helpers
// -----------------------------

static QByteArray generateToneWav(double frequency, int durationMs, double volume = 0.25)
{
    const int sampleRate  = 44100;
    const int sampleCount = (sampleRate * durationMs) / 1000;

    QByteArray data;
    data.resize(sampleCount * 2); // 16-bit mono
    qint16* samples = reinterpret_cast<qint16*>(data.data());

    const double twoPiF      = 2.0 * M_PI * frequency;
    const double fadeSamples = sampleRate * 0.005; // 5 ms fade

    for (int i = 0; i < sampleCount; ++i) {
        double t      = double(i) / sampleRate;
        double sample = qSin(twoPiF * t);

        // Fade-in
        if (i < fadeSamples)
            sample *= (double(i) / fadeSamples);

        // Fade-out
        if (i > sampleCount - fadeSamples)
            sample *= double(sampleCount - i) / fadeSamples;

        samples[i] = qint16(sample * 32767.0 * volume);
    }

    // Build WAV header
    QByteArray wav;
    wav.reserve(44 + data.size());

    auto write32 = [&](quint32 v) {
        wav.append(char(v & 0xFF));
        wav.append(char((v >> 8) & 0xFF));
        wav.append(char((v >> 16) & 0xFF));
        wav.append(char((v >> 24) & 0xFF));
    };

    auto write16 = [&](quint16 v) {
        wav.append(char(v & 0xFF));
        wav.append(char((v >> 8) & 0xFF));
    };

    // RIFF header
    wav.append("RIFF");
    write32(36 + data.size());
    wav.append("WAVE");

    // fmt chunk
    wav.append("fmt ");
    write32(16);
    write16(1);        // PCM
    write16(1);        // mono
    write32(sampleRate);
    write32(sampleRate * 2);
    write16(2);
    write16(16);

    // data chunk
    wav.append("data");
    write32(data.size());
    wav.append(data);

    return wav;
}

static QByteArray generateTwoToneWav(double f1, double f2, int durationMs, double volume = 0.25)
{
    const int half = durationMs / 2;
    QByteArray a   = generateToneWav(f1, half, volume);
    QByteArray b   = generateToneWav(f2, half, volume);

    QByteArray wav = a;
    wav.append(b.mid(44)); // skip second header
    return wav;
}

static QByteArray cachedTone(double freq, int ms, double vol = 0.25)
{
    QString key = QString("%1_%2_%3").arg(freq).arg(ms).arg(vol);
    static QMap<QString, QByteArray> cache;

    if (!cache.contains(key))
        cache[key] = generateToneWav(freq, ms, vol);

    return cache[key];
}

static QByteArray cachedTwoTone(double f1, double f2, int ms, double vol = 0.25)
{
    QString key = QString("2_%1_%2_%3_%4").arg(f1).arg(f2).arg(ms).arg(vol);
    static QMap<QString, QByteArray> cache;

    if (!cache.contains(key))
        cache[key] = generateTwoToneWav(f1, f2, ms, vol);

    return cache[key];
}
// -----------------------------
// AACFeedbackEngine methods
// -----------------------------

void AACFeedbackEngine::initSounds()
{
    auto load = [&](QSoundEffect& eff, const QByteArray& wav) {
        auto* buf = new QBuffer(this);
        buf->setData(wav);
        buf->open(QIODevice::ReadOnly);
        eff.setSource(QUrl());
        eff.setLoopCount(1);
        eff.setVolume(0.9f); // base volume; will be overridden per‑play for fatigue
        eff.setSourceDevice(buf);
    };

    // Core UI
    load(m_clickSound,  cachedTone(660.0, 30));
    load(m_focusSound,  cachedTone(880.0, 40));
    load(m_errorSound,  cachedTone(330.0, 120));
    load(m_dwellSound,  cachedTwoTone(523.25, 660.0, 120));

    // Messaging
    load(m_incomingMessageSound, cachedTone(523.25, 60));
    load(m_privateMessageSound,  cachedTone(440.0, 60));

    // User presence + transmit
    load(m_userJoinSound,  cachedTone(660.0, 50));
    load(m_userLeaveSound, cachedTone(440.0, 50));
    load(m_transmitOnSound,  cachedTone(700.0, 40));
    load(m_transmitOffSound, cachedTone(500.0, 40));

    // Polish earcons
    load(m_screenChangeSound,      cachedTone(880.0, 60));
    load(m_messageSentSound,       cachedTone(1320.0, 40));
    load(m_adjustSound,            cachedTone(660.0, 40));
    load(m_predictiveUpdateSound,  cachedTone(990.0, 40));
    load(m_modeChangeSound,        cachedTone(1040.0, 70));
    load(m_panelToggleSound,       cachedTone(780.0, 50));
    load(m_connectionLostSound,    cachedTone(220.0, 120));
    load(m_connectionRestoredSound,cachedTone(1760.0, 80));
    load(m_serverErrorSound,       cachedTone(330.0, 120));

// Backspace: lower-pitched, very short click
load(m_backspaceSound, cachedTone(1800.0, 4));

// Enter: confirmation sweep (two-tone)
load(m_enterSound, cachedTwoTone(1200.0, 1800.0, 30));

// Action: neutral tick (Clear, Delete Word, Move Cursor)
load(m_actionSound, cachedTone(2000.0, 6));
}

void AACFeedbackEngine::playEffectWithFatigue(QSoundEffect& eff,
                                              float normalVol,
                                              float fatigueVol)
{
    if (!m_mgr || !m_mgr->modes().auditoryFeedback)
        return;

    float volume = m_mgr->modes().fatigueMode ? fatigueVol : normalVol;

    // Base attenuation for scanning sounds
    if (m_mgr->modes().reduceScanningSoundIntensity &&
        isScanningSound(eff))
    {
        volume *= 0.4f;

        // Adaptive: faster scanning → quieter ticks
        const int speed = m_mgr->modes().scanningSpeedMs;
if (speed < 900)
    volume *= 0.8f;
if (speed < 700)
    volume *= 0.65f;
        if (speed < 500)
            volume *= 0.5f;
    }

    volume = qBound(0.0f, volume, 1.0f);
    eff.setVolume(volume);
    eff.play();
}
bool AACFeedbackEngine::isScanningSound(const QSoundEffect& eff) const
{
    return (&eff == &m_focusSound ||
            &eff == &m_dwellSound ||
            &eff == &m_clickSound);
}
// -----------------------------
// Haptics
// -----------------------------

static int mapSemanticToStrength(HapticSemantic semantic,
                                 bool fatigue,
                                 bool talkBack)
{
    int base = 0;
    switch (semantic) {
    case HapticSemantic::Background: base = 1; break;
    case HapticSemantic::Confirm:   base = 2; break;
    case HapticSemantic::Error:     base = 3; break;
    }

    if (fatigue && !talkBack && base > 1)
        base -= 1;

    if (talkBack)
        base = qMin(base + 1, 3);

    return qBound(1, base, 3);
}
void AACFeedbackEngine::doHaptic(int strength, HapticSemantic semantic)
{
    const auto& m = m_mgr->modes();

    // --- Platform screenreader suppression ---
    // If ANY screenreader is active, suppress ALL haptics EXCEPT Confirm/Error.
    if (m.talkBackMode || m.voiceOverMode || m.switchControlScanning) {
        if (semantic == HapticSemantic::Background)
            return;    // suppress scan ticks
    }

    // --- AAC fatigue mode: suppress background haptics ---
    if (m.fatigueMode && semantic == HapticSemantic::Background)
        return;

    // --- Global feedback mode: suppress all haptics ---
    if (m.globalFeedback)
        return;

    // --- Map semantic → numeric strength (fatigue + TalkBack shaping) ---
    strength = mapSemanticToStrength(
        semantic,
        m.fatigueMode,
        m.talkBackMode
    );

    // --- Platform-specific dispatch ---
    doHaptic(strength);
}
void AACFeedbackEngine::doHaptic(int strength)
{
    if (!m_mgr)
        return;

    // Subtoggle: user disabled haptics
    if (!m_mgr->modes().hapticFeedback)
        return;

    // --- Fatigue shaping ---
    // If fatigue mode is on (and not TalkBack), soften strong haptics
    if (!m_mgr->modes().talkBackMode && m_mgr->modes().fatigueMode) {
        if (strength > 1)
            strength -= 1;
    }

    // --- TalkBack/VoiceOver boosting ---
    // If TalkBack is active, boost haptics one level
    if (m_mgr->modes().talkBackMode) {
        strength = qMin(strength + 1, 3);
    }

    // Clamp
    strength = qBound(1, strength, 3);

    bool delivered = false;

#if defined(Q_OS_WIN)
    // Try WinRT first (Surface trackpads, modern actuators)
    if (HapticBackend_Windows::performWinRT(strength)) {
        delivered = true;
    }
    // If WinRT didn't deliver, try XInput (controllers, rumble motors)
    else {
        HapticBackend_Windows::perform(strength);
        delivered = true; // XInput always "delivers" if a controller exists
    }

#elif defined(Q_OS_IOS)
    delivered = HapticBackend_iOS::perform(strength);

#elif defined(Q_OS_MACOS)
    delivered = HapticBackend_macOS::perform(strength);

#elif defined(Q_OS_ANDROID)
    delivered = HapticBackend_Android::perform(strength);

#elif defined(Q_OS_LINUX)
    delivered = HapticBackend_Linux::perform(strength);

#else
    Q_UNUSED(strength);
#endif

    // --- Fallback: visual pulse + click ---
    if (!delivered) {
        m_clickSound.play();
        emit visualPulseRequested(strength);
    }
}

void AACFeedbackEngine::hapticSoft()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    doHaptic(1, HapticSemantic::Background);
}

void AACFeedbackEngine::hapticStrong()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    doHaptic(2, HapticSemantic::Confirm);
}

void AACFeedbackEngine::hapticError()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    doHaptic(3, HapticSemantic::Error);
}
// -----------------------------
// Core UI sounds
// -----------------------------

void AACFeedbackEngine::playClick()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_clickSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playFocus()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_focusSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playError()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_errorSound);

    if (m_mgr->modes().hapticFeedback)
        hapticError();
}

void AACFeedbackEngine::playDwellComplete()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_dwellSound);

    if (m_mgr->modes().hapticFeedback)
        hapticStrong();
}
void AACFeedbackEngine::playStepAdvance()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_focusSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

// -----------------------------
// Messaging
// -----------------------------

void AACFeedbackEngine::playIncomingMessage()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_incomingMessageSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playPrivateMessage()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_privateMessageSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}


// -----------------------------
// User join/leave + transmit
// -----------------------------

void AACFeedbackEngine::playUserJoin()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_userJoinSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playUserLeave()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_userLeaveSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playTransmitOn()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_transmitOnSound);

    if (m_mgr->modes().hapticFeedback)
        hapticStrong();
}

void AACFeedbackEngine::playTransmitOff()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_transmitOffSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}


// -----------------------------
// Polish earcons
// -----------------------------

void AACFeedbackEngine::playScreenChange()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_screenChangeSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playMessageSent()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_messageSentSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playAdjust()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_adjustSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playPredictiveUpdate()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_predictiveUpdateSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playModeChange()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_modeChangeSound);

    if (m_mgr->modes().hapticFeedback)
        hapticStrong();
}

void AACFeedbackEngine::playPanelToggle()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_panelToggleSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playBackspace()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_backspaceSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playEnter()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_enterSound);

    if (m_mgr->modes().hapticFeedback)
        hapticStrong();
}

void AACFeedbackEngine::playAction()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_actionSound);

    if (m_mgr->modes().hapticFeedback)
        hapticSoft();
}

void AACFeedbackEngine::playConnectionLost()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_connectionLostSound);

    if (m_mgr->modes().hapticFeedback)
        hapticError();
}

void AACFeedbackEngine::playConnectionRestored()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_connectionRestoredSound);

    if (m_mgr->modes().hapticFeedback)
        hapticStrong();
}

void AACFeedbackEngine::playServerError()
{
    if (!m_mgr->modes().feedbackEnabled)
        return;

    if (m_mgr->modes().auditoryFeedback)
        playEffectWithFatigue(m_serverErrorSound);

    if (m_mgr->modes().hapticFeedback)
        hapticError();
}

AACSpeechEngine::AACSpeechEngine(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
    m_tts = new QTextToSpeech(this);
    connect(m_tts, &QTextToSpeech::stateChanged,
            this, [this](QTextToSpeech::State st) {
                if (st == QTextToSpeech::Speaking)
                    emit speechStarted(QString());
                else if (st == QTextToSpeech::Ready)
                    emit speechFinished(QString());
            });
}

bool AACSpeechEngine::shouldSpeakScanning() const
{
    const AACModeFlags m = m_mgr->modes();

    // --- Platform screenreader suppression ---
    if (m.talkBackMode)
        return false;

    if (m.voiceOverMode)
        return false;

    if (m.switchControlScanning)
        return false;

    // --- AAC fatigue mode: scanning speech becomes softer or suppressed ---
    if (m.fatigueMode)
        return false;

    // --- Global feedback mode: visual pulses only ---
    if (m.globalFeedback)
        return false;

    // --- User preference: auditory feedback must be enabled ---
    return m.auditoryFeedback;
}
void AACSpeechEngine::speak(const QString& text)
{
    if (!m_tts)
        return;

    AACSpeechConfig cfg = m_cfg;
    bool fatigue = m_mgr->modes().fatigueMode;

    // ============================================================
    // 1. Fatigue‑aware shaping preset
    // ============================================================
    if (fatigue) {
        cfg.rate   *= 0.85;
        cfg.pitch  *= 0.90;
        cfg.volume *= 0.80;
        cfg.intonationRange *= 0.85;
        cfg.sibilanceSmoothing += 0.10f;
    }

    // ============================================================
    // 2. Fatigue‑aware reading mode (long utterances)
    // ============================================================
    bool longText = text.length() > 40;
    if (fatigue && longText) {
        cfg.rate   *= 0.80;
        cfg.pitch  *= 0.90;
        cfg.volume *= 0.85;
        cfg.intonationRange *= 0.80;
    }

    // Apply shaping
    m_tts->setRate(cfg.rate);
    m_tts->setPitch(cfg.pitch);

    // ============================================================
    // Fade‑in (two‑step, AAC‑safe)
    // ============================================================
    if (fatigue) {
        m_tts->setVolume(cfg.volume * 0.3);
        m_tts->say(text);

        QTimer::singleShot(90, [this, cfg]() {
            if (m_tts)
                m_tts->setVolume(cfg.volume * 0.7);
        });

        QTimer::singleShot(180, [this, cfg]() {
            if (m_tts)
                m_tts->setVolume(cfg.volume);
        });

    } else {
        m_tts->setVolume(cfg.volume);
        m_tts->say(text);
    }
}

void AACSpeechEngine::speakNotification(const QString& text)
{
    if (!m_tts)
        return;

    AACSpeechConfig cfg = m_cfg;
    bool fatigue = m_mgr->modes().fatigueMode;

    // 1. Fatigue‑aware shaping preset
    if (fatigue) {
        cfg.rate   *= 0.85;
        cfg.pitch  *= 0.90;
        cfg.volume *= 0.80;
        cfg.intonationRange *= 0.85;
        cfg.sibilanceSmoothing += 0.10f;
    }

    // 2. Notification‑specific softening
    if (fatigue) {
        cfg.rate   *= 0.90;
        cfg.pitch  *= 0.85;
        cfg.volume *= 0.75;
    }

    m_tts->setRate(cfg.rate);
    m_tts->setPitch(cfg.pitch);

    // 3. Fade‑in
    if (fatigue) {
        m_tts->setVolume(cfg.volume * 0.3);
        m_tts->say(text);

        QTimer::singleShot(90, [this, cfg]() {
            if (m_tts)
                m_tts->setVolume(cfg.volume * 0.7);
        });

        QTimer::singleShot(180, [this, cfg]() {
            if (m_tts)
                m_tts->setVolume(cfg.volume);
        });

    } else {
        m_tts->setVolume(cfg.volume);
        m_tts->say(text);
    }
}
void AACSpeechEngine::speakNotification(const QString& text, SpeechPriority p)
{
    // If currently speaking lower priority → interrupt
    if (m_speaking && p < m_currentPriority) {
        stop();
        m_speaking = false;
    }

    // If speaking higher priority → queue
    if (m_speaking && p > m_currentPriority) {
        m_queue.enqueue({text, p});
        return;
    }

    // Speak immediately
    m_currentPriority = p;
    m_speaking = true;

    emit speechStarted(text);

    // ⭐ Call your existing fatigue‑aware renderer
    speakNotification(text);

    // When TTS finishes, continue queue
    connect(m_tts, &QTextToSpeech::stateChanged, this,
            [this, text](QTextToSpeech::State state) {
        if (state == QTextToSpeech::Ready) {
            m_speaking = false;
            emit speechFinished(text);
            speakQueued();
        }
    });
}
void AACSpeechEngine::speakScanningRow(const QString& label)
{
    if (!m_mgr || !m_tts)
        return;

    if (!shouldSpeakScanning())
        return;

    AACSpeechConfig cfg = m_cfg;
    cfg.rate += 0.1;
    cfg.volume *= 0.8;
    lowIntensityShaping(cfg);

    m_tts->setRate(cfg.rate);
    m_tts->setPitch(cfg.pitch);
    m_tts->setVolume(cfg.volume);

    m_tts->say(label);
}

void AACSpeechEngine::speakScanningItem(const QString& label)
{
    if (!m_mgr || !m_tts)
        return;

    if (!shouldSpeakScanning())
        return;

    AACSpeechConfig cfg = m_cfg;
    cfg.rate += 0.05;
    cfg.volume *= 0.9;
    lowIntensityShaping(cfg);

    m_tts->setRate(cfg.rate);
    m_tts->setPitch(cfg.pitch);
    m_tts->setVolume(cfg.volume);

    m_tts->say(label);
}
void AACSpeechEngine::speakQueued()
{
    if (m_queue.isEmpty())
        return;

    SpeechItem next = m_queue.dequeue();
    speakNotification(next.text, next.priority);
}
void AACSpeechEngine::stop()
{
    if (!m_tts)
        return;
    m_tts->stop();
}

void AACSpeechEngine::setVoice(const QString& voiceName)
{
    if (!m_tts)
        return;

    const auto voices = m_tts->availableVoices();
    for (const QVoice& v : voices) {
        if (v.name() == voiceName) {
            m_tts->setVoice(v);
            break;
        }
    }
}

void AACSpeechEngine::setRate(double rate)
{
    if (!m_tts)
        return;
    m_tts->setRate(rate);
}

void AACSpeechEngine::setPitch(double pitch)
{
    if (!m_tts)
        return;
    m_tts->setPitch(pitch);
}

void AACSpeechEngine::speakLetter(const QString& letter)
{
    if (!m_tts)
        return;
    if (m_sayMode != AACSpeechConfig::SpeakLetters)
        return;
    if (letter.trimmed().isEmpty())
        return;
    speak(letter);
}

void AACSpeechEngine::intelligibilityShaping(AACSpeechConfig& cfg)
{
    if (!cfg.highIntelligible)
        return;

    if (cfg.rate > 1.0)
        cfg.rate = 1.0 + (cfg.rate - 1.0) * 0.5;
    if (cfg.pitch < 1.0)
        cfg.pitch = cfg.pitch + (1.0 - cfg.pitch) * 0.3;
    if (cfg.volume < 0.7)
        cfg.volume = 0.7;
}

void AACSpeechEngine::lowIntensityShaping(AACSpeechConfig& cfg)
{
    if (!cfg.lowIntensity)
        return;
    if (cfg.volume > 0.6)
        cfg.volume = 0.6;
    if (cfg.volume < 0.3)
        cfg.volume = 0.3;
    cfg.pitch = cfg.pitch * 0.9;
    cfg.rate = cfg.rate * 0.9;
}

void AACSpeechEngine::applyPresetShaping(AACSpeechConfig& cfg)
{
    Q_UNUSED(cfg);
    // Placeholder for preset-based shaping.
}

void AACSpeechEngine::applyConfig(const AACSpeechConfig& cfg)
{
    if (!m_tts)
        return;

    AACSpeechConfig effective = cfg;
    applyPresetShaping(effective);
    intelligibilityShaping(effective);
    lowIntensityShaping(effective);

    if (effective.rate < 0.1)
        effective.rate = 0.1;
    if (effective.rate > 2.0)
        effective.rate = 2.0;

    if (effective.pitch < 0.1)
        effective.pitch = 0.1;
    if (effective.pitch > 2.0)
        effective.pitch = 2.0;

    if (effective.volume < 0.0)
        effective.volume = 0.0;
    if (effective.volume > 1.0)
        effective.volume = 1.0;

    m_cfg = effective;

    if (!m_cfg.voiceName.isEmpty()) {
        const auto voices = m_tts->availableVoices();
        for (const QVoice& v : voices) {
            if (v.name() == m_cfg.voiceName) {
                m_tts->setVoice(v);
                break;
            }
        }
    }

    m_tts->setRate(m_cfg.rate);
    m_tts->setPitch(m_cfg.pitch);
    m_tts->setVolume(m_cfg.volume);
    m_sayMode = m_cfg.speakAsYouTypeMode;
}

void AACSpeechEngine::speakWord(const QString& word)
{
    if (!m_tts)
        return;
    if (m_sayMode != AACSpeechConfig::SpeakWords)
        return;
    if (word.trimmed().isEmpty())
        return;
    speak(word);
}

void AACSpeechEngine::speakPhrase(const QString& phrase)
{
    if (!m_tts)
        return;
    if (m_sayMode != AACSpeechConfig::SpeakPhrases)
        return;
    if (phrase.trimmed().isEmpty())
        return;
    speak(phrase);
}

void AACSpeechEngine::echoOnSend(const QString& text)
{
    if (!m_cfg.echoOnSend)
        return;
    if (text.trimmed().isEmpty())
        return;
    speak(text);
}

AACMessageHistory::AACMessageHistory(AACAccessibilityManager* mgr, QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
}

void AACMessageHistory::addMessage(const QString& msg)
{
    if (msg.trimmed().isEmpty())
        return;

    m_history << msg;
    emit historyChanged(m_history);
}

QStringList AACMessageHistory::history() const
{
    return m_history;
}

void AACMessageHistory::replayMessage(int index)
{
    if (!m_mgr || index < 0 || index >= m_history.size())
        return;

    const QString msg = m_history.at(index);
    if (AACSpeechEngine* s = m_mgr->speechEngine())
        s->speak(msg);
}

void AACMessageHistory::replayLast()
{
    if (!m_mgr || m_history.isEmpty())
        return;

    const QString msg = m_history.last();
    if (AACSpeechEngine* s = m_mgr->speechEngine())
        s->speak(msg);
}
#include "AACFramework.h"
#include "AACKeyboardScreen.h"
#include "AACPredictionEngine.h"

AACFramework::AACFramework(QObject* parent)
    : QObject(parent)
{
    m_accessibility = new AACAccessibilityManager(this);
    m_keyboardScreen = new AACKeyboardScreen(m_accessibility);

    // Prediction → symbol highlight
    connect(m_accessibility->predictionEngine(),
            &AACPredictionEngine::semanticContextChanged,
            m_keyboardScreen,
            &AACKeyboardScreen::setSemanticHighlight);

    // Symbol tapped → semantic feedback
    connect(m_keyboardScreen,
            &AACKeyboardScreen::symbolSemantic,
            m_accessibility->predictionEngine(),
            &AACPredictionEngine::onSymbolChosen);

    // Word predictions → predictive strip
    connect(m_accessibility->predictionEngine(),
            &AACPredictionEngine::predictionsUpdated,
            m_keyboardScreen,
            &AACKeyboardScreen::setPredictions);

    // Prediction chosen → engine learns
    connect(m_keyboardScreen,
            &AACKeyboardScreen::predictionChosen,
            m_accessibility->predictionEngine(),
            &AACPredictionEngine::onPredictionChosen);

    // Typed characters → prediction context
    connect(m_keyboardScreen,
            &AACKeyboardScreen::characterTyped,
            m_accessibility->predictionEngine(),
            &AACPredictionEngine::onCharacterTyped);
}
// --------------------------------------------------------
// Modes → keyboard (core symbols / curated strip dwell)
// --------------------------------------------------------
{
    const AACModeFlags modes = m_accessibility->modes();
    m_keyboardScreen->setCoreSymbolsFirst(modes.coreSymbolsFirst);
    m_keyboardScreen->setCuratedStripDwellEnabled(modes.curatedStripDwell);
}

connect(m_accessibility, &AACAccessibilityManager::modesChanged,
        m_keyboardScreen,
        [this](const AACModeFlags& modes) {
            m_keyboardScreen->setCoreSymbolsFirst(modes.coreSymbolsFirst);
            m_keyboardScreen->setCuratedStripDwellEnabled(modes.curatedStripDwell);
        });
