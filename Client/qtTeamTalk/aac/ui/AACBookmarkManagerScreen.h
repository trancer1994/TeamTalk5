#pragma once
#include "AACScreenBase.h"
#include "AACConversationRecorder.h"
#include "AACKeyButton.h"

class AACBookmarkManagerScreen : public AACScreenBase
{
    Q_OBJECT
public:
    AACBookmarkManagerScreen(AACConversationRecorderQtAdapter* recorder,
                             AACAccessibilityManager* aac,
                             QWidget* parent = nullptr);

signals:
    void requestJumpToIndex(int index);

private:
    AACConversationRecorderQtAdapter* m_recorder = nullptr;
    QVBoxLayout* m_layout = nullptr;
};
