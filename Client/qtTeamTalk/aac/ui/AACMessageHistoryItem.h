#pragma once

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "AACMessageHistoryEvent.h"
#include "AACKeyButton.h"

class AACMessageHistoryItem : public QWidget
{
    Q_OBJECT

public:
    explicit AACMessageHistoryItem(const AACMessageHistoryEvent& ev,
                                   QWidget* parent = nullptr);

    QLabel* textLabel() const { return m_textLabel; }
    AACKeyButton* replayButton() const { return m_replayBtn; }
    AACKeyButton* speakButton() const { return m_speakBtn; }
    AACKeyButton* copyButton() const { return m_copyBtn; }
    AACKeyButton* deleteButton() const { return m_deleteBtn; }

signals:
    void requestReplay(const AACMessageHistoryEvent& ev);
    void requestSpeak(const AACMessageHistoryEvent& ev);
    void requestCopy(const AACMessageHistoryEvent& ev);
    void requestDelete(const AACMessageHistoryEvent& ev);

private:
    AACMessageHistoryEvent m_event;

    QLabel* m_textLabel = nullptr;
    QLabel* m_timestampLabel = nullptr;
    QLabel* m_semanticLabel = nullptr;

    AACKeyButton* m_replayBtn = nullptr;
    AACKeyButton* m_speakBtn = nullptr;
    AACKeyButton* m_copyBtn = nullptr;
    AACKeyButton* m_deleteBtn = nullptr;
};
