#pragma once
#include "AACScreenBase.h"
#include "AACCommunicationMethodScreen.h"
#include "AACTypingMethodScreen.h"

struct AACProfileConfig {
    CommMethod   commMethod;
    TypingMethod typingMethod;
    QString      vocabId;
bool helpMode = false;
};

class AACSetupSummaryScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit AACSetupSummaryScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setConfig(const AACProfileConfig& cfg);

signals:
    void setupComplete(const AACProfileConfig& cfg);

private:
    AACProfileConfig m_cfg;
AACToggle* m_helpModeToggle = nullptr;
};
