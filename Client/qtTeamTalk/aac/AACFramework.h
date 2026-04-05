#pragma once

#include <QPushButton>
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

#include <memory>

#include "storage/aacstorage.h"
#include "core/AACProfile.h"
#include "core/AACProfileConfigTable.h"

class QWidget;
class QLayout;
class QEnterEvent;
class QFocusEvent;
class QMouseEvent;
class QPaintEvent;
class QTextToSpeech;

class AACLayoutEngine;
class AACInputController;
class AACFeedbackEngine;
class AACSpeechEngine;
class AACMessageHistory;
class AACVocabularyManager;
class AACPredictionEngine;
class AACScreenAdapter;

class AACAccessibilityManager : public QObject
{
    Q_OBJECT

public:
    AACAccessibilityManager(QObject* parent = nullptr);

    void hydrate();
    void persist();
    void attachToAppLifecycle(QObject* app);

    QString activeCategory() const;
    AACProfile profile() const;
    AACModeFlags modes() const;
    AACDwellConfig dwellConfig() const;
    AACScanningConfig scanningConfig() const;
    AACLayoutConfig layoutConfig() const;
    AACSpeechConfig speechConfig() const;
    bool predictionEnabled() const;

    AACInputController* inputController() const;
    AACFeedbackEngine* feedbackEngine() const;
    AACSpeechEngine* speechEngine() const;
    AACMessageHistory* history() const;
    AACPredictionEngine* predictionEngine() const;
    AACVocabularyManager* vocabularyManager() const;

public slots:
    void setActiveCategory(const QString& category);
    void setProfile(AACProfile profile);
    void setModes(const AACModeFlags& modes);
    void setDwellConfig(const AACDwellConfig& config);
    void setScanningConfig(const AACScanningConfig& config);
    void setLayoutConfig(const AACLayoutConfig& config);
    void setSpeechConfig(const AACSpeechConfig& config);
    void setPredictionEnabled(bool enabled);

    void boostPredictionVocabulary();
    void loadPredictionForUser(const QString& userId);
    void savePredictionForUser(const QString& userId);

signals:
    void activeCategoryChanged(const QString& category);
    void profileChanged(AACProfile profile);
    void modesChanged(const AACModeFlags& modes);
    void dwellConfigChanged(const AACDwellConfig& config);
    void scanningConfigChanged(const AACScanningConfig& config);
    void layoutConfigChanged(const AACLayoutConfig& config);
    void speechConfigChanged(const AACSpeechConfig& config);
    void predictionEnabledChanged(bool enabled);
    void speechStarted(const QString& text);
    void speechFinished(const QString& text);
    void historyChanged(const QStringList& history);

private:
    QString m_activeCategory;
    AACProfile m_profile = AACProfile::CoreVocabulary;

    AACModeFlags m_modes;
    AACDwellConfig m_dwellConfig;
    AACScanningConfig m_scanningConfig;
    AACLayoutConfig m_layoutConfig;

    AACLayoutEngine* m_layoutEngine = nullptr;
    AACInputController* m_inputController = nullptr;
    AACFeedbackEngine* m_feedbackEngine = nullptr;

    AACSpeechEngine* m_speechEngine = nullptr;
    AACMessageHistory* m_history = nullptr;
    AACVocabularyManager* m_vocabularyManager = nullptr;

    AACPredictionEngine* m_predictionEngine = nullptr;
    bool m_predictionEnabled = true;

    AACSpeechConfig m_speechConfig;

    std::unique_ptr<AACStorage> m_storage;
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

class AACInputController : public QObject
{
    Q_OBJECT

public:
    AACInputController(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void attachScreen(AACScreenAdapter* screen);
    void detachScreen(AACScreenAdapter* screen);

    void startDwellOn(QWidget* w);
    void stopDwellOn(QWidget* w);

signals:
    void dwellProgressChanged(QWidget* target, float progress);
    void dwellActivated(QWidget* target);
    void scanningFocused(QWidget* target);
    void deepWellActivated(QWidget* target);

public slots:
    void onSwitchActivate();
    void onSwitchNext();

private slots:
    void onDwellTick();
    void onScanningTick();

private:
    void rebuildScanningList();
    void startDwell(QWidget* w);
    void stopDwell();
    void focusWidget(QWidget* w);
    bool isDeepWell(QWidget* w) const;
    void activateWidget(QWidget* w);

    AACAccessibilityManager* m_mgr = nullptr;
    AACScreenAdapter* m_currentScreen = nullptr;

    QList<QPointer<QWidget>> m_scanningWidgets;
    int m_scanningIndex = -1;

    QTimer m_dwellTimer;
    QElapsedTimer m_dwellElapsed;
    QPointer<QWidget> m_dwellTarget;

    QTimer m_scanningTimer;
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

    void hapticSoft();
    void hapticStrong();
    void hapticError();

private:
    void initSounds();
    void doHaptic(int strength);

    AACAccessibilityManager* m_mgr = nullptr;

    QSoundEffect m_clickSound;
    QSoundEffect m_focusSound;
    QSoundEffect m_errorSound;
    QSoundEffect m_dwellSound;
};

class AACButton : public QPushButton
{
    Q_OBJECT

public:
    AACButton(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setDeepWell(bool enabled);
    bool isDeepWell() const;

    void setDwellProgress(float p);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    AACAccessibilityManager* m_aac = nullptr;
    bool m_deepWell = false;
    float m_dwellProgress = 0.0f;
};

class AACSpeechEngine : public QObject
{
    Q_OBJECT

public:
    AACSpeechEngine(AACAccessibilityManager* mgr, QObject* parent = nullptr);

    void speak(const QString& text);
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

    AACAccessibilityManager* m_mgr = nullptr;
    QTextToSpeech* m_tts = nullptr;
    AACSpeechConfig m_cfg;
    AACSpeechConfig::SpeakAsYouTypeMode m_sayMode = AACSpeechConfig::SpeakNone;
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
