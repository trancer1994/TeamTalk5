#pragma once

#include "AACMessage.h"

// A thin wrapper so the history viewer has a stable, UI-friendly type.
// Internally it is just an AACMessage.
struct AACMessageHistoryEvent
{
    AACMessage msg;

    AACMessageHistoryEvent() = default;
    AACMessageHistoryEvent(const AACMessage& m)
        : msg(m)
    {}
};
