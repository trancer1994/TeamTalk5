#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>

struct AACMessage
{
    QString text;

    bool    isPrivate   = false;
    int     channelId   = 0;
    int     toUserId    = 0;
    QString toUsername;

    int     fromUserId  = 0;
    QString fromUsername;

    QString semanticTag;
    QString vocabId;
    QStringList tags;

    QDateTime timestamp;

    static AACMessage channel(const QString& text,
                              int channelId,
                              const QString& fromUser = QString(),
                              int fromUserId = 0)
    {
        AACMessage m;
        m.text         = text;
        m.isPrivate    = false;
        m.channelId    = channelId;
        m.fromUsername = fromUser;
        m.fromUserId   = fromUserId;
        m.timestamp    = QDateTime::currentDateTime();
        return m;
    }

    static AACMessage privateMsg(const QString& text,
                                 const QString& toUser,
                                 int toUserId = 0)
    {
        AACMessage m;
        m.text       = text;
        m.isPrivate  = true;
        m.toUsername = toUser;
        m.toUserId   = toUserId;
        m.timestamp  = QDateTime::currentDateTime();
        return m;
    }
};
