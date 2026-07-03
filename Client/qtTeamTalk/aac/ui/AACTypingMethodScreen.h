#pragma once
#include "AACScreenBase.h"

enum class TypingMethod { KeyboardOnly, SymbolGrid, Both };

class AACTypingMethodScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit AACTypingMethodScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void typingMethodChosen(TypingMethod method);
};
