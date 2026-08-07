#include "AACJumpLastAudioFrameButton.h"

AACJumpLastAudioFrameButton::AACJumpLastAudioFrameButton(QWidget* parent)
    : AACKeyButton(parent)
{
    setText(tr("Jump to last audio frame"));
    setAccessibleName(tr("Jump to last audio frame"));

    connect(this, &AACKeyButton::activated, this, [this]() {
        emit jumpToLastAudioFrame();
    });
}
