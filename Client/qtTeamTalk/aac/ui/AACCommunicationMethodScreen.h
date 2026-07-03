#pragma once
#include "AACScreenBase.h"

enum class CommMethod { TouchClick, GazeDwell, SwitchScanning };

class AACCommunicationMethodScreen : public AACScreenBase
{
    Q_OBJECT
public:
    AACCommunicationMethodScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void communicationMethodChosen(CommMethod method);
};
