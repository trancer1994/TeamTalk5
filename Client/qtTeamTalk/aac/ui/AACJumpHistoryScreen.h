#pragma once
#include "AACScreenBase.h"
#include "AACKeyButton.h"

class AACJumpHistoryScreen : public AACScreenBase
{
    Q_OBJECT
public:
    AACJumpHistoryScreen(const QVector<int>& history,
                         const QVector<AACConversationRecorderQtAdapter::Event>& events,
                         AACAccessibilityManager* aac,
                         QWidget* parent = nullptr);

signals:
    void requestJumpToIndex(int index);
};
