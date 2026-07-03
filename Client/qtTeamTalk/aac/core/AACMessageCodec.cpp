#include "aac/core/AACMessageCodec.h"
#include <cstring>
#include "TeamTalk.h"

namespace AACMessageCodec
{
    void encodeToTeamTalk(const AACMessage& aac,
                          TTTextMessage& tt,
                          TTInstance* ttInstance)
    {
        Q_UNUSED(ttInstance);
        memset(&tt, 0, sizeof(tt));

        const QByteArray utf8 = aac.text.toUtf8();
        qstrncpy(tt.szMessage, utf8.constData(), sizeof(tt.szMessage));

        tt.nChannelID = aac.channelId;
        tt.nToUserID  = aac.toUserId;
    }

    AACMessage decodeFromTeamTalk(const TTTextMessage& tt,
                                  const QString& fromUsername)
    {
        AACMessage aac;

        aac.text         = QString::fromUtf8(tt.szMessage);
        aac.fromUserId   = tt.nFromUserID;
        aac.fromUsername = fromUsername;
        aac.channelId    = tt.nChannelID;
        aac.timestamp    = QDateTime::currentDateTime();

        return aac;
    }
}
