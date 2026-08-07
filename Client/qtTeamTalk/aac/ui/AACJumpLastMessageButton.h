#pragma once
#include "AACKeyButton.h"

class AACJumpLastMessageButton : public AACKeyButton
{
    Q_OBJECT

public:
    AACJumpLastMessageButton(QWidget* parent = nullptr);

signals:
    void jumpToLastAACMessage();
};
