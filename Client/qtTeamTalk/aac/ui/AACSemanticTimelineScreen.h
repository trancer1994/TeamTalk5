#pragma once
#include "AACScreenBase.h"
#include "AACConversationRecorderQtAdapter.h"
#include "AACKeyButton.h"

class AACSemanticTimelineScreen : public AACScreenBase
{
    Q_OBJECT
public:
    AACSemanticTimelineScreen(AACConversationRecorderQtAdapter* recorder,
                              AACAccessibilityManager* aac,
                              QWidget* parent = nullptr);

signals:
    void requestJumpToIndex(int index);
};
