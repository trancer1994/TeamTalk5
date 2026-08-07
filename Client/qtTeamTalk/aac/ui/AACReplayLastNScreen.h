#pragma once

#include "AACScreenBase.h"
#include "AACConversationRecorderQtAdapter.h"
#include "AACKeyButton.h"

class AACReplayLastNScreen : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACReplayLastNScreen(AACConversationRecorderQtAdapter* recorder,
                                  AACAccessibilityManager* aac,
                                  QWidget* parent = nullptr);

signals:
    void requestReplayEvents(const QVector<AACConversationRecorderQtAdapter::Event>& events);

private:
    AACConversationRecorderQtAdapter* m_recorder = nullptr;

    AACKeyButton* m_buttonLast3 = nullptr;
    AACKeyButton* m_buttonLast5 = nullptr;
    AACKeyButton* m_buttonLast10 = nullptr;

    AACKeyButton* m_buttonLastSpoken = nullptr;
    AACKeyButton* m_buttonLastAAC = nullptr;
    AACKeyButton* m_buttonLastAudioFrame = nullptr;

    AACKeyButton* m_buttonClose = nullptr;

    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
};
