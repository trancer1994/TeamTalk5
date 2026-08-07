#pragma once

#include "AACScreenBase.h"
#include "AACConversationRecorderQtAdapter.h"
#include "AACKeyButton.h"

class AACEventMetadataScreen : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACEventMetadataScreen(const AACConversationRecorderQtAdapter::Event& ev,
                                    AACAccessibilityManager* aac,
                                    QWidget* parent = nullptr);

signals:
    void requestReplayEvent(const AACConversationRecorderQtAdapter::Event& ev);
    void requestExportEvent(const AACConversationRecorderQtAdapter::Event& ev);
    void requestJumpToEvent(const AACConversationRecorderQtAdapter::Event& ev);
    void requestToggleBookmark(const AACConversationRecorderQtAdapter::Event& ev);

private:
    AACConversationRecorderQtAdapter::Event m_event;

    // UI elements
    AACKeyButton* m_labelTimestamp = nullptr;
    AACKeyButton* m_labelType = nullptr;
    AACKeyButton* m_labelText = nullptr;
    AACKeyButton* m_labelDirection = nullptr;
AACWaveformPreview* m_waveform = nullptr;

    AACKeyButton* m_buttonReplay = nullptr;
    AACKeyButton* m_buttonExport = nullptr;
    AACKeyButton* m_buttonClose = nullptr;
    AACKeyButton* m_jumpToBookmarkButton = nullptr;
    AACKeyButton* m_bookmarkToggleButton = nullptr;
bool m_isBookmarked = false;

    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
};
