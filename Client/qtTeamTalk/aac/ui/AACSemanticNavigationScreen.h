#pragma once

#include "AACScreenBase.h"
#include "AACConversationRecorderQtAdapter.h"
#include "AACKeyButton.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QString>

class AACSemanticNavigationScreen : public AACScreenBase
{
    Q_OBJECT
public:
    AACSemanticNavigationScreen(AACConversationRecorderQtAdapter* rec,
                                AACAccessibilityManager* aac,
                                QWidget* parent = nullptr)
        : AACScreenBase(aac, parent)
        , m_rec(rec)
    {
        setScreenTitle(tr("Semantic Navigation"));

        auto* root = new QVBoxLayout(this);

        auto addJump = [&](const QString& label, const QString& tag) {
            auto* btn = new AACKeyButton(this);
            btn->setText(label);
            connect(btn, &AACKeyButton::activated, this, [this, tag]() {
                int idx = m_rec->nextEventWithTag(tag, 0);
                emit requestJumpToIndex(idx);
            });
            root->addWidget(btn);
        };

        addJump(tr("Next sad event"), "emotion_sad");
        addJump(tr("Next happy event"), "emotion_happy");
        addJump(tr("Next need_help event"), "need_help");
        addJump(tr("Next urgent event"), "urgent");

        root->addStretch(1);
    }

signals:
    void requestJumpToIndex(int index);

private:
    AACConversationRecorderQtAdapter* m_rec;
};
