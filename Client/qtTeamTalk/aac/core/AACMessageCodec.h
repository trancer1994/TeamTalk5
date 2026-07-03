#pragma once

#include <QString>
#include "aac/models/AACMessage.h"

struct TTTextMessage;
struct TTInstance;

namespace AACMessageCodec
{
    void encodeToTeamTalk(const AACMessage& aac,
                          TTTextMessage& tt,
                          TTInstance* ttInstance);

    AACMessage decodeFromTeamTalk(const TTTextMessage& tt,
                                  const QString& fromUsername);
}
