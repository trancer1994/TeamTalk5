#pragma once

#include <QWidget>
#include <QList>
#include <QVBoxLayout>
#include <QScrollArea>

#include "AACScreenBase.h"
#include "AACMessageHistory.h"
#include "AACMessageHistoryEvent.h"
#include "AACKeyButton.h"

class AACMessageHistoryItem;

class AACMessageHistoryViewer : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACMessageHistoryViewer(AACMessageHistory* history,
                                     QWidget* parent = nullptr);

    // AACScreenAdapter overrides
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QString contextualHelp() const override;

private slots:
    void onHistoryChanged(const QList<AACMessage>& list);

private:
    AACMessageHistory* m_history = nullptr;

    QVBoxLayout* m_rootLayout = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_scrollWidget = nullptr;
    QVBoxLayout* m_listLayout = nullptr;

    QList<AACMessageHistoryItem*> m_items;
};
