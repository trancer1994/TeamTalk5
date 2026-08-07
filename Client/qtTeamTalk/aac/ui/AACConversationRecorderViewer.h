#pragma once

#include <QScrollArea>
#include <QVBoxLayout>
#include "AACScreenBase.h"
#include "AACConversationRecorderQtAdapter.h"
#include "AACConversationRecorderItem.h"

class AACConversationRecorderViewer : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACConversationRecorderViewer(AACConversationRecorderQtAdapter* recorder,
                                           AACAccessibilityManager* aac,
                                           QWidget* parent = nullptr);

    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QString contextualHelp() const override;

    void refreshScanning();

signals:
    // Play entire conversation audio
    void requestPlayConversation();

    // Show metadata for a specific event
    void requestShowDetails(const AACConversationRecorder::Event& ev);

    // Export entire conversation audio
    void requestExportConversation();

private slots:
    void onConversationUpdated(const QVector<AACConversationRecorder::Event>& events);

private:
    AACConversationRecorderQtAdapter* m_recorder = nullptr;

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_scrollWidget = nullptr;
    QVBoxLayout* m_listLayout = nullptr;

    QList<AACConversationRecorderItem*> m_items;
QList<AACEventSeekButton*> m_seekButtons;
AACJumpLastMessageButton* m_jumpLastMessage = nullptr;
AACJumpLastAudioFrameButton* m_jumpLastAudioFrame = nullptr;
AACKeyButton* m_summaryButton = nullptr;
AACKeyButton* m_replayOptionsButton = nullptr;
AACKeyButton* m_exportOptionsButton = nullptr;
void scrollToIndex(int index);
void scrollToEvent(const AACConversationRecorder::Event& ev);
    QVector<int> m_jumpHistory;
    AACKeyButton* m_semanticNavButton = nullptr;
    AACKeyButton* m_bookmarkManagerButton = nullptr;
    AACKeyButton* m_jumpHistoryButton = nullptr;
    AACKeyButton* m_semanticTimelineButton = nullptr;
};
