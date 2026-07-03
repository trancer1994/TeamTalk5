#include "AACVocabularyScreen.h"
#include "AACKeyButton.h"
#include <QVBoxLayout>

AACVocabularyScreen::AACVocabularyScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("Which vocabulary set do you want for the grid?"));

    auto* layout = new QVBoxLayout(this);

    struct V { QString id; QString label; };
    const QList<V> sets = {
        { "core",  tr("Core vocabulary") },
        { "topic", tr("Topic vocabulary") },
        { "custom", tr("Custom vocabulary") }
    };

    for (const auto& v : sets) {
        auto* btn = new AACKeyButton(aac, v.label, this);
        layout->addWidget(btn);
        connect(btn, &AACKeyButton::activated, this, [this, v]() {
            emit vocabularyChosen(v.id);
        });
    }

    setLayout(layout);
}
