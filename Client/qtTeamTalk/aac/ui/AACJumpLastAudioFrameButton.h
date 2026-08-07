#pragma once
#include "AACKeyButton.h"

class AACJumpLastAudioFrameButton : public AACKeyButton
{
    Q_OBJECT

public:
    AACJumpLastAudioFrameButton(QWidget* parent = nullptr);

signals:
    void jumpToLastAudioFrame();
};
