#pragma once

#include "AACScreenBase.h"
#include "AACConversationRecorderQtAdapter.h"
#include "AACKeyButton.h"

class AACExportOptionsScreen : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACExportOptionsScreen(AACConversationRecorderQtAdapter* recorder,
                                    AACAccessibilityManager* aac,
                                    QWidget* parent = nullptr);

signals:
    void requestExportTranscript(const QString& text);
    void requestExportEventList(const QByteArray& json);
    void requestExportSummary(const QString& text);

private:
    AACConversationRecorder* m_recorder = nullptr;

    AACKeyButton* m_buttonTranscript = nullptr;
    AACKeyButton* m_buttonEventList = nullptr;
    AACKeyButton* m_buttonSummary = nullptr;
    AACKeyButton* m_buttonClose = nullptr;

    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
};
