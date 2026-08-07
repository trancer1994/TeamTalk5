#include "AACSemanticTimelineScreen.h"
#include <QVBoxLayout>

AACSemanticTimelineScreen::AACSemanticTimelineScreen(
        AACConversationRecorderQtAdapter* recorder,
        AACAccessibilityManager* aac,
        QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("Semantic timeline"));

    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    const auto& events = recorder->events();

    for (int i = 0; i < events.size(); ++i) {
        const auto& ev = events[i];
        if (ev.semanticTag.isEmpty())
            continue;

        auto* row = new AACKeyButton(this);
        row->setText(tr("%1 — %2 (%3)")
                     .arg(ev.timestamp.toString("hh:mm:ss"))
                     .arg(ev.semanticTag)
                     .arg(ev.text.isEmpty() ? tr("(no text)") : ev.text.left(40)));
        row->setAccessibleName(tr("Semantic event %1 at %2")
                               .arg(ev.semanticTag,
                                    ev.timestamp.toString("hh:mm:ss")));
        connect(row, &AACKeyButton::activated, this, [this, i]() {
            emit requestJumpToIndex(i);
        });
        root->addWidget(row);
    }

    auto* closeButton = new AACKeyButton(this);
    closeButton->setText(tr("Close"));
    closeButton->setAccessibleName(tr("Close semantic timeline"));
    connect(closeButton, &AACKeyButton::activated, this, [this]() {
        close();
    });
    root->addWidget(closeButton);

    root->addStretch(1);
}
