#pragma once

#include "AACScreenBase.h"
#include "AACConversationRecorder.h"
#include "AACKeyButton.h"

class AACConversationSummaryScreen : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACConversationSummaryScreen(
        const QVector<AACConversationRecorder::Event>& events,
        AACAccessibilityManager* aac,
        QWidget* parent = nullptr);

signals:
    void requestReplayConversation();
    void requestExportConversation();

private:
    QVector<AACConversationRecorder::Event> m_events;

    AACKeyButton* m_labelTotal = nullptr;
    AACKeyButton* m_labelAACSent = nullptr;
    AACKeyButton* m_labelAACSpoken = nullptr;
    AACKeyButton* m_labelAudioToUser = nullptr;
    AACKeyButton* m_labelAudioFromUser = nullptr;
    AACKeyButton* m_labelDuration = nullptr;

    AACKeyButton* m_buttonReplay = nullptr;
    AACKeyButton* m_buttonExport = nullptr;
    AACKeyButton* m_buttonClose = nullptr;

    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
};
