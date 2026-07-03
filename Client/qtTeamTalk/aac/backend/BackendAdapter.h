#pragma once

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QString>
#include <QList>

#include "aac/models/AACMessage.h"
#include "BackendEvents.h"
#include "aac/core/AACFramework.h"   // Needed for feedback + accessibility

class BackendAdapter : public QObject {
    Q_OBJECT

public:
    explicit BackendAdapter(QObject* parent = nullptr);

    // Connection
    void connectToServer(const QString& host, int port, const QString& username);
    void reconnectLastServer();
    void disconnectFromServer();

    // Channel operations
    void refreshChannels();
    void joinChannel(int channelId);
    void leaveChannel();

    // Voice
    enum class AACTransmitMode {
        TapToToggle,
        Continuous,
        VoiceActivation,
        AutoSilence
    };

    void setTransmitMode(AACTransmitMode mode);
    AACTransmitMode transmitMode() const { return m_mode; }
    void setTransmitEnabled(bool enabled);

    // Auto‑silence helpers
    void resetAutoSilence();        // called on speech / activity
    void endAutoSilenceSession();   // called on Done / Clear

    // Text messaging
    void sendChannelMessage(const AACMessage& msg);
    void sendPrivateMessage(const AACMessage& msg);

    // Event pump
    void processEvents();

    // AACFramework wiring
    void setAACFramework(AACFramework* fw) { m_aacFramework = fw; }

signals:
    // Backend → StateMachine
    void transmitStateChanged(bool enabled);
    void channelsEnumerated(const QList<ChannelInfo>& channels);
    void connectionStateChanged(ConnectionState state);
    void channelEvent(const ChannelEvent& event);
    void backendError(const ErrorEvent& error);
    void channelForcedLeave(const QString& reason);
    void selfVoiceEvent(const SelfVoiceEvent& event);
    void otherUserVoiceEvent(const OtherUserVoiceEvent& event);

    // AAC text message event (channel + private)
    void aacMessageReceived(const AACMessage& msg);

private:
    // TeamTalk instance
    TTInstance* m_tt = nullptr;

    // Transmit mode
    AACTransmitMode m_mode = AACTransmitMode::TapToToggle;

    // Auto‑silence state
    QTimer* m_autoSilenceTimer = nullptr;
    int m_autoSilenceElapsedMs = 0;
    static constexpr int AutoSilenceTimeoutMs = 300000; // 5 minutes

    // Last connection info
    QString m_lastHost;
    int     m_lastPort = 0;
    QString m_lastUsername;

    // User tracking
    struct UserInfo {
        int userId = 0;
        QString username;
        int channelId = 0;
    };
    QList<UserInfo> m_users;

    bool m_userInitiatedLeave = false;

    // AAC framework (for feedback + accessibility)
    AACFramework* m_aacFramework = nullptr;

    // Helpers
    QString mapErrorCode(int code, const QString& raw);
    void log(const QString& message);

    int findUserIndex(int userId) const;
    int findUserIndexByName(const QString& name) const;
    QString usernameForId(int userId) const;
    int resolveUserId(const QString& username) const;   // REQUIRED for username-based private messaging
    void upsertUser(int userId, const QString& username, int channelId);
    void removeUser(int userId);

    int refVolume(int percent) const; // convert 0–100% to TT volume

    struct {
        QString username;
    } m_state;

private slots:
    void onAutoSilenceTick();
};
