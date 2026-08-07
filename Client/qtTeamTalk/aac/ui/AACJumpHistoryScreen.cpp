#include "AACJumpHistoryScreen.h"
#include <QVBoxLayout>

AACJumpHistoryScreen::AACJumpHistoryScreen(
        const QVector<int>& history,
        const QVector<AACConversationRecorderQtAdapter::Event>& events,
        AACAccessibilityManager* aac,
        QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("Jump history"));

    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    for (int idx : history) {
        if (idx < 0 || idx >= events.size())
            continue;
        const auto& ev = events[idx];

        auto* row = new AACKeyButton(this);
        row->setText(tr("Jumped to %1 — %2")
                     .arg(ev.timestamp.toString("hh:mm:ss"))
                     .arg(ev.text.isEmpty() ? tr("(no text)") : ev.text.left(40)));
        row->setAccessibleName(tr("Jump history entry at %1").arg(ev.timestamp.toString("hh:mm:ss")));
        connect(row, &AACKeyButton::activated, this, [this, idx]() {
            emit requestJumpToIndex(idx);
        });
        root->addWidget(row);
    }

    auto* clearButton = new AACKeyButton(this);
    clearButton->setText(tr("Clear history"));
    clearButton->setAccessibleName(tr("Clear jump history"));
    connect(clearButton, &AACKeyButton::activated, this, [this]() {
        close();
    });
    root->addWidget(clearButton);

    auto* closeButton = new AACKeyButton(this);
    closeButton->setText(tr("Close"));
    closeButton->setAccessibleName(tr("Close jump history"));
    connect(closeButton, &AACKeyButton::activated, this, [this]() {
        close();
    });
    root->addWidget(closeButton);

    root->addStretch(1);
}
