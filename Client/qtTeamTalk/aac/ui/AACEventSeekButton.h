#pragma once
#include "AACKeyButton.h"
#include "AACConversationRecorderQtAdapter.h"

class AACEventSeekButton : public AACKeyButton
{
    Q_OBJECT

public:
AACEventSeekButton(const AACConversationRecorderQtAdapter::Event& ev,
                       int index,
                       QWidget* parent = nullptr);

signals:
    void seekToEvent(int index);
    void showEventDetails(const AACConversationRecorder::Event& ev);

private:
AACConversationRecorderQtAdapter::Event m_event;
    int m_index;
};
