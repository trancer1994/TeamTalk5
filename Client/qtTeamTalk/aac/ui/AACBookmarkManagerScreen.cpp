#include "AACBookmarkManagerScreen.h"
#include <QVBoxLayout>

AACBookmarkManagerScreen::AACBookmarkManagerScreen(
        AACConversationRecorderQtAdapter* recorder,
        AACAccessibilityManager* aac,
        QWidget* parent)
    : AACScreenBase(aac, parent)
    , m_recorder(recorder)
{
    setScreenTitle(tr("Bookmarks"));

    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    const auto indices = m_recorder->bookmarkedEvents();
    const auto& events = m_recorder->events();

    for (int idx : indices) {
        if (idx < 0 || idx >= events.size())
            continue;
        const auto& ev = events[idx];

        auto* row = new AACKeyButton(this);
        row->setText(tr("%1 — %2")
                     .arg(ev.timestamp.toString("hh:mm:ss"))
                     .arg(ev.text.isEmpty() ? tr("(no text)") : ev.text.left(40)));
        row->setAccessibleName(tr("Bookmark at %1").arg(ev.timestamp.toString("hh:mm:ss")));
        connect(row, &AACKeyButton::activated, this, [this, idx]() {
            emit requestJumpToIndex(idx);
        });
        root->addWidget(row);
    }

    auto* clearButton = new AACKeyButton(this);
    clearButton->setText(tr("Remove all bookmarks"));
    clearButton->setAccessibleName(tr("Remove all bookmarks"));
    connect(clearButton, &AACKeyButton::activated, this, [this]() {
        const auto indices = m_recorder->bookmarkedEvents();
        for (int idx : indices)
            m_recorder->removeBookmark(idx);
        close();
    });
    root->addWidget(clearButton);

    auto* closeButton = new AACKeyButton(this);
    closeButton->setText(tr("Close"));
    closeButton->setAccessibleName(tr("Close bookmark manager"));
    connect(closeButton, &AACKeyButton::activated, this, [this]() {
        close();
    });
    root->addWidget(closeButton);

    root->addStretch(1);
}
