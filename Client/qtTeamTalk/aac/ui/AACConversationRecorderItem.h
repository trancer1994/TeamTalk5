#pragma once
#include "AACKeyButton.h"
#include "AACConversationRecorderQtAdapter.h"

class AACConversationRecorderItem : public AACKeyButton
{
    Q_OBJECT

public:
    AACConversationRecorderItem(const AACConversationRecorder::Event& ev,
                                QWidget* parent = nullptr);

signals:
    void requestPlay();
    void requestDetails(const AACConversationRecorder::Event& ev);
    void requestExport();

private:
    AACConversationRecorder::Event m_event;
QString m_timestamp;
};
