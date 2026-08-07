#pragma once
#include "AACKeyButton.h"
#include "AACConversationRecorderQtAdapter.h"

class AACConversationRecorderItem : public AACKeyButton
{
    Q_OBJECT

public:
    AACConversationRecorderItem(const AACConversationRecorderQtAdapter::Event& ev,
                                QWidget* parent = nullptr);

signals:
    void requestPlay();
    void requestDetails(const AACConversationRecorderQtAdapter::Event& ev);
    void requestExport();

private:
    AACConversationRecorderQtAdapter::Event m_event;
QString m_timestamp;
};
