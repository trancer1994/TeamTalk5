#include "BackendAdapter.h"
#include "aac/core/AACMessageCodec.h"
#include "aac/core/AACFramework.h"
#include <QTextStream>
#include <cstring>
#include "TeamTalk.h"

BackendAdapter::BackendAdapter(QObject* parent)
    : QObject(parent)
{
    // Initialise TeamTalk instance
    m_tt = TT_InitTeamTalkPoll();

    // Auto‑silence timer
    m_autoSilenceTimer = new QTimer(this);
    m_autoSilenceTimer->setInterval(1000); // 1s tick
    connect(m_autoSilenceTimer, &QTimer::timeout,
            this, &BackendAdapter::onAutoSilenceTick);
}

//
// ------------------------------------------------------------
// Connection
// ------------------------------------------------------------
//

void BackendAdapter::connectToServer(const QString& host, int port, const QString& username)
{
    if (!m_tt)
        return;

    m_lastHost     = host;
    m_lastPort     = port;
    m_lastUsername = username;
    m_state.username = username;

    log(QString("Connecting to %1:%2 as %3")
        .arg(host).arg(port).arg(username));

    TT_Connect(
        m_tt,
        host.toUtf8().constData(),
        port,
        0, 0,
        username.toUtf8().constData(),
        ""
    );
}

void BackendAdapter::reconnectLastServer()
{
    if (m_lastHost.isEmpty() || m_lastPort == 0 || m_lastUsername.isEmpty())
        return;

    connectToServer(m_lastHost, m_lastPort, m_lastUsername);
}

void BackendAdapter::disconnectFromServer()
{
    if (!m_tt)
        return;

    log("Disconnecting from server");
    TT_Disconnect(m_tt);
    m_users.clear();
}

//
// ------------------------------------------------------------
// Channel operations
// ------------------------------------------------------------
//

void BackendAdapter::refreshChannels()
{
    if (!m_tt)
        return;

    log("Requesting channel list");
    TT_DoChannelList(m_tt);
}

void BackendAdapter::joinChannel(int channelId)
{
    if (!m_tt)
        return;

    log(QString("Joining channel %1").arg(channelId));
    TT_JoinChannelByID(m_tt, channelId, "");
}

void BackendAdapter::leaveChannel()
{
    if (!m_tt)
        return;

    m_userInitiatedLeave = true;
    log("Leaving channel");
    TT_LeaveChannel(m_tt);
}

//
// ------------------------------------------------------------
// Voice
// ------------------------------------------------------------
//

void BackendAdapter::setTransmitMode(AACTransmitMode mode)
{
    if (!m_tt)
        return;

    // If leaving Auto‑Silence, ensure transmit is OFF
    if (m_mode == AACTransmitMode::AutoSilence &&
        mode != AACTransmitMode::AutoSilence)
    {
        m_autoSilenceTimer->stop();
        m_autoSilenceElapsedMs = 0;
        setTransmitEnabled(false);
    }

    m_mode = mode;

    // Reset Auto‑Silence state
    m_autoSilenceTimer->stop();
    m_autoSilenceElapsedMs = 0;

    switch (mode) {

    case AACTransmitMode::TapToToggle:
        TT_EnableVoiceActivation(m_tt, false);
        setTransmitEnabled(false);
        break;

    case AACTransmitMode::Continuous:
        TT_EnableVoiceActivation(m_tt, false);
        setTransmitEnabled(true);
        break;

    case AACTransmitMode::VoiceActivation:
        TT_EnableVoiceActivation(m_tt, true);
        setTransmitEnabled(false);
        break;

    case AACTransmitMode::AutoSilence:
        TT_EnableVoiceActivation(m_tt, false);
        setTransmitEnabled(true);
        m_autoSilenceElapsedMs = 0;
        m_autoSilenceTimer->start();
        break;
    }
}

void BackendAdapter::setTransmitEnabled(bool enabled)
{
    if (!m_tt)
        return;

    log(QString("Transmit %1").arg(enabled ? "ON" : "OFF"));

    // AAC feedback for transmit on/off
    if (m_aacFramework &&
        m_aacFramework->feedbackEngine() &&
        !m_aacFramework->modes().fatigueMode)
    {
        if (enabled)
            m_aacFramework->feedbackEngine()->playTransmitOn();
        else
            m_aacFramework->feedbackEngine()->playTransmitOff();
    }

    emit transmitStateChanged(enabled);

    TT_SetTransmissionMode(
        m_tt,
        enabled ? TRANSMIT_AUDIO : TRANSMIT_NONE
    );
}

void BackendAdapter::onAutoSilenceTick()
{
    if (!m_tt)
        return;

    if (m_mode != AACTransmitMode::AutoSilence)
        return;

    m_autoSilenceElapsedMs += m_autoSilenceTimer->interval();

    if (m_autoSilenceElapsedMs >= AutoSilenceTimeoutMs) {
        log("Auto‑silence timeout reached, stopping transmit");
        setTransmitEnabled(false);
        m_autoSilenceTimer->stop();
    }
}

void BackendAdapter::resetAutoSilence()
{
    if (!m_tt)
        return;

    if (m_mode != AACTransmitMode::AutoSilence)
        return;

    m_autoSilenceElapsedMs = 0;

    // In Auto‑Silence, we assume the user is still in a “speaking session”.
    // Ensure transmit is ON while they are active.
    setTransmitEnabled(true);

    if (!m_autoSilenceTimer->isActive())
        m_autoSilenceTimer->start();
}

void BackendAdapter::endAutoSilenceSession()
{
    if (!m_tt)
        return;

    if (m_mode != AACTransmitMode::AutoSilence)
        return;

    log("Auto‑silence session ended explicitly");
    m_autoSilenceTimer->stop();
    m_autoSilenceElapsedMs = 0;
    setTransmitEnabled(false);
}

//
// ------------------------------------------------------------
// Text messaging
// ------------------------------------------------------------
//

void BackendAdapter::sendChannelMessage(const AACMessage& aac)
{
    if (!m_tt)
        return;

    TTTextMessage msg;
    std::memset(&msg, 0, sizeof(msg));

    AACMessageCodec::encodeToTeamTalk(aac, msg, m_tt);

    if (msg.nChannelID == 0)
        msg.nChannelID = TT_GetMyChannelID(m_tt);

    msg.nMsgType = MSGTYPE_CHANNEL;

    TT_DoTextMessage(m_tt, &msg);
}

void BackendAdapter::sendPrivateMessage(const AACMessage& aac)
{
    if (!m_tt)
        return;

    int targetUserId = aac.toUserId;

    // Fallback: resolve by username if ID is not set
    if (targetUserId == 0 && !aac.toUsername.isEmpty())
        targetUserId = resolveUserId(aac.toUsername);

    if (targetUserId == 0)
        return;

    TTTextMessage msg;
    std::memset(&msg, 0, sizeof(msg));

    AACMessageCodec::encodeToTeamTalk(aac, msg, m_tt);

    msg.nMsgType  = MSGTYPE_USER;
    msg.nToUserID = targetUserId;

    TT_DoTextMessage(m_tt, &msg);
}

//
// ------------------------------------------------------------
// Event pump
// ------------------------------------------------------------
//

void BackendAdapter::processEvents()
{
    if (!m_tt)
        return;

    TTMessage msg;
    while (TT_GetMessage(m_tt, &msg)) {

        switch (msg.nClientEvent) {

        // --------------------------------------------------------
        // Connection state
        // --------------------------------------------------------
        case CLIENTEVENT_CON_CONNECTING:
            emit connectionStateChanged(ConnectionState::Connecting);
            break;

        case CLIENTEVENT_CON_SUCCESS:
        {
            emit connectionStateChanged(ConnectionState::Connected);

            // Persist last successful connection into AAC layer
            if (m_aacFramework && m_aacFramework->accessibilityManager()) {
                auto* acc = m_aacFramework->accessibilityManager();
                acc->setLastHost(m_lastHost);
                acc->setLastPort(static_cast<quint16>(m_lastPort));
                acc->setLastUsername(m_lastUsername);
                // Optional: acc->setLastPassword(...);
            }

            // Optional: connection restored earcon
            if (m_aacFramework && m_aacFramework->earcons())
                m_aacFramework->earcons()->play(Earcon::ConnectionRestored);

            break;
        }

        case CLIENTEVENT_CON_FAILED:
        case CLIENTEVENT_CON_LOST:
            emit connectionStateChanged(ConnectionState::Disconnected);
            m_users.clear();
            break;

        // --------------------------------------------------------
        // Channel list
        // --------------------------------------------------------
        case CLIENTEVENT_CMD_CHANNEL_LIST:
        {
            QList<ChannelInfo> list;

            for (int i = 0; i < msg.channel.nChannels; ++i) {
                const TTChannel* ch = msg.channel.lpChannels[i];

                ChannelInfo info;
                info.id       = ch->nChannelID;
                info.parentId = ch->nParentID;
                info.name     = QString::fromUtf8(ch->szName);

                list.append(info);
            }

            emit channelsEnumerated(list);
            break;
        }

        // --------------------------------------------------------
        // Join / leave channel (self)
        // --------------------------------------------------------
        case CLIENTEVENT_CMD_MYSELF_JOINED_CHANNEL:
        {
            if (m_aacFramework &&
                m_aacFramework->feedbackEngine() &&
                !m_aacFramework->modes().fatigueMode)
            {
                m_aacFramework->feedbackEngine()->playUserJoin();
            }

            ChannelEvent ev;
            ev.type      = ChannelEventType::Joined;
            ev.channelId = msg.channel.nChannelID;
            emit channelEvent(ev);
            break;
        }

        case CLIENTEVENT_CMD_MYSELF_LEFT_CHANNEL:
        {
            bool userLeft = m_userInitiatedLeave;
            m_userInitiatedLeave = false; // reset for next time

            if (userLeft) {
                // User pressed “Leave channel”
                if (m_aacFramework &&
                    m_aacFramework->feedbackEngine() &&
                    !m_aacFramework->modes().fatigueMode)
                {
                    m_aacFramework->feedbackEngine()->playUserLeave();
                }

                ChannelEvent ev;
                ev.type      = ChannelEventType::Left;
                ev.channelId = msg.channel.nChannelID;
                emit channelEvent(ev);
            }
            else {
                // Server forced us out
                QString reason = QStringLiteral("Channel closed");
                emit channelForcedLeave(reason);
            }

            break;
        }

        // --------------------------------------------------------
        // User join/leave (others) for tracking
        // --------------------------------------------------------
        case CLIENTEVENT_CMD_USER_JOINED:
        {
            int userId = msg.user.nUserID;
            QString nick = QString::fromUtf8(msg.user.szNickname);
            int chId = msg.user.nChannelID;
            upsertUser(userId, nick, chId);

            // AAC: restore per-user volume on join (keyed by username)
            if (m_aacFramework && m_aacFramework->accessibilityManager()) {
                auto* acc = m_aacFramework->accessibilityManager();
                auto vols = acc->allUserVolumes();
                if (vols.contains(nick)) {
                    int percent = vols.value(nick);
                    int vol = refVolume(percent);
                    TT_SetUserVolume(m_tt, userId, STREAMTYPE_VOICE, vol);
                }
            }

            break;
        }

        case CLIENTEVENT_CMD_USER_LEFT:
        {
            int userId = msg.user.nUserID;
            removeUser(userId);
            break;
        }

        // --------------------------------------------------------
        // Errors
        // --------------------------------------------------------
        case CLIENTEVENT_CMD_ERROR:
        {
            ErrorEvent err;
            err.message = mapErrorCode(
                msg.clienterrormsg.nErrorNo,
                QString::fromUtf8(msg.clienterrormsg.szErrorMsg)
            );

            log(QString("Error: %1").arg(err.message));
            emit backendError(err);
            break;
        }

        // --------------------------------------------------------
        // Voice state (self + others)
// --------------------------------------------------------
        case CLIENTEVENT_USER_STATECHANGE:
        {
            int myId = TT_GetMyUserID(m_tt);

            // Track user info
            {
                int userId = msg.user.nUserID;
                QString nick = QString::fromUtf8(msg.user.szNickname);
                int chId = msg.user.nChannelID;
                upsertUser(userId, nick, chId);
            }

            // Self
            if (msg.user.nUserID == myId) {
                SelfVoiceEvent ev;
                ev.state = (msg.user.uUserState & USERSTATE_VOICE)
                    ? SelfVoiceState::Transmitting
                    : SelfVoiceState::Silent;

                emit selfVoiceEvent(ev);
            }
            // Others
            else {
                OtherUserVoiceEvent ev;
                ev.userId   = msg.user.nUserID;
                ev.username = QString::fromUtf8(msg.user.szNickname);
                ev.state    = (msg.user.uUserState & USERSTATE_VOICE)
                    ? OtherUserVoiceState::Speaking
                    : OtherUserVoiceState::Silent;

                emit otherUserVoiceEvent(ev);
            }
            break;
        }

        // --------------------------------------------------------
        // Text messages (channel + private)
// --------------------------------------------------------
        case CLIENTEVENT_CMD_USER_TEXTMSG:
        case CLIENTEVENT_CMD_CHANNEL_TEXTMSG:
        {
            const TTTextMessage& tm = msg.textmsg;

            AACMessage aac = AACMessageCodec::decodeFromTeamTalk(
                tm,
                usernameForId(tm.nFromUserID)
            );

            if (msg.nClientEvent == CLIENTEVENT_CMD_CHANNEL_TEXTMSG) {
                aac.isPrivate = false;
                aac.channelId = tm.nChannelID;
            } else {
                aac.isPrivate = true;
                aac.channelId = 0;
            }

            emit aacMessageReceived(aac);
            break;
        }

        default:
            break;
        }
    }
}

//
// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
//

QString BackendAdapter::mapErrorCode(int code, const QString& raw)
{
    switch (code) {

    case TT_CMDERR_TIMEOUT:
        return "The server did not respond. Please check your connection.";

    case TT_CMDERR_SERVER_FULL:
        return "The server is full. Try again later.";

    case TT_CMDERR_NOT_LOGGEDIN:
        return "You are not connected to the server.";

    case TT_CMDERR_CHANNEL_NOT_FOUND:
        return "The channel no longer exists.";

    case TT_CMDERR_ALREADY_IN_CHANNEL:
        return "You are already in this channel.";

    case TT_CMDERR_CHANNEL_FULL:
        return "The channel is full.";

    case TT_CMDERR_CHANNEL_BANNED:
        return "You are banned from this channel.";

    case TT_CMDERR_CHANNEL_KICKED:
        return "You were removed from the channel.";

    case TT_CMDERR_CHANNEL_PASSWORD:
        return "Incorrect password.";

    case TT_CMDERR_INVALID_USERNAME:
        return "Invalid username.";

    default:
        return raw.isEmpty()
            ? "An unknown error occurred."
            : raw;
    }
}

void BackendAdapter::log(const QString& message)
{
    QFile f("backend.log");
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out << QDateTime::currentDateTime().toString(Qt::ISODate)
            << " - " << message << "\n";
    }
}

int BackendAdapter::findUserIndex(int userId) const
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].userId == userId)
            return i;
    }
    return -1;
}

int BackendAdapter::findUserIndexByName(const QString& name) const
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].username == name)
            return i;
    }
    return -1;
}

QString BackendAdapter::usernameForId(int userId) const
{
    int idx = findUserIndex(userId);
    if (idx >= 0)
        return m_users[idx].username;

    // Fallback: ask TT directly if our table is stale
    TTUser* u = nullptr;
    if (TT_GetUser(m_tt, userId, &u) && u)
        return QString::fromUtf8(u->szNickname);

    return QString();
}

void BackendAdapter::upsertUser(int userId, const QString& username, int channelId)
{
    int idx = findUserIndex(userId);
    if (idx >= 0) {
        m_users[idx].username = username;
        m_users[idx].channelId = channelId;
    } else {
        UserInfo info;
        info.userId   = userId;
        info.username = username;
        info.channelId = channelId;
        m_users.append(info);
    }
}

void BackendAdapter::removeUser(int userId)
{
    int idx = findUserIndex(userId);
    if (idx >= 0)
        m_users.removeAt(idx);
}

int BackendAdapter::resolveUserId(const QString& username) const
{
    int idx = findUserIndexByName(username);
    if (idx >= 0)
        return m_users[idx].userId;

    if (!m_tt)
        return 0;

    int userCount = TT_GetUserCount(m_tt);
    for (int i = 0; i < userCount; ++i) {
        int userId = TT_GetUserID(m_tt, i);
        TTUser* u = nullptr;
        if (TT_GetUser(m_tt, userId, &u) && u) {
            QString nick = QString::fromUtf8(u->szNickname);
            if (nick == username)
                return userId;
        }
    }
    return 0;
}

int BackendAdapter::refVolume(int percent) const
{
    // Simple mapping: 0–100% → 0–100 TT volume
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return percent;
}
