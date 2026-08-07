#include "AACJumpLastMessageButton.h"

AACJumpLastMessageButton::AACJumpLastMessageButton(QWidget* parent)
    : AACKeyButton(parent)
{
    setText(tr("Jump to last AAC message"));
    setAccessibleName(tr("Jump to last AAC message"));

    connect(this, &AACKeyButton::activated, this, [this]() {
        emit jumpToLastAACMessage();
    });
}
