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

class AACLayoutEngine;
class AACInputController;
class AACFeedbackEngine;
class AACSpeechEngine;
class AACMessageHistory;
class AACVocabularyManager;
class AACPredictionEngine;

class AACAccessibilityManager : public QObject
{
    Q_OBJECT

public:
    explicit AACAccessibilityManager(QObject* parent = nullptr);

    void hydrate();
    void persist();

    QString activeCategory() const;
    AACProfile profile() const;
    AACModeFlags modes() const;
    AACDwellConfig dwellConfig() const;
    AACScanningConfig scanningConfig() const;
    AACLayoutConfig layoutConfig() const;
    AACSpeechConfig speechConfig() const;
    bool predictionEnabled() const;

public slots:
    void setActiveCategory(const QString& category);
    void setProfile(AACProfile profile);
    void setModes(const AACModeFlags& modes);
    void setDwellConfig(const AACDwellConfig& config);
    void setScanningConfig(const AACScanningConfig& config);
    void setLayoutConfig(const AACLayoutConfig& config);
    void setSpeechConfig(const AACSpeechConfig& config);
    void setPredictionEnabled(bool enabled);

signals:
    void activeCategoryChanged(const QString& category);
    void profileChanged(AACProfile profile);
    void modesChanged(const AACModeFlags& modes);
    void dwellConfigChanged(const AACDwellConfig& config);
    void scanningConfigChanged(const AACScanningConfig& config);
    void layoutConfigChanged(const AACLayoutConfig& config);
    void speechConfigChanged(const AACSpeechConfig& config);
    void predictionEnabledChanged(bool enabled);

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
