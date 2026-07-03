#pragma once
#include <QObject>
#include <QElapsedTimer>
#include "aac/AACFramework.h"

// -------------------------
// Event + Priority enums
// -------------------------

enum class AACEvent {
    None,
    NavigateForward,
    NavigateBack,
    FocusInitialised,
    Activate,
    Confirm,
    Error,
    MessageReceived,
    ChannelJoin,
    ChannelLeave,
    UserJoin,
    UserLeave,
    ReconnectAttempt,
    ReconnectFailed,
    ReconnectSuccess,
    MessageSent,     // NEW
    Help
};

enum class AACPriority {
    High,       // Errors
    Help,       // Help earcon
    Medium,     // Confirm
    Nav,        // Navigation + reconnect
    Low,        // Activation + message sent
    VeryLow     // Focus + message received + join/leave
};

// -------------------------
// Router class
// -------------------------

class AACEarconRouter : public QObject
{
    Q_OBJECT
public:
    explicit AACEarconRouter(AACFramework* aac, QObject* parent = nullptr);

    // Navigation
    void navForward();
    void navBack();

    // Focus
    void focusInit();

    // Activation
    void activate();

    // Confirmation
    void confirm();

    // Errors
    void error();

    // Messaging
    void messageReceived();
    void messageSent();     // NEW

    // Channel events
    void channelJoin();
    void channelLeave();
    void userJoin();
    void userLeave();

    // Reconnect events
    void reconnectAttempt();
    void reconnectFailed();
    void reconnectSuccess();

    // Help
    void help();

private:
    bool shouldSuppress(AACEvent ev, int minIntervalMs);
    bool priorityAllows(AACPriority p);
    void play(AACEvent ev, AACPriority p);

private:
    AACFramework* m_aac;
    QElapsedTimer m_timer;

    AACEvent m_lastEvent = AACEvent::None;
    qint64 m_lastTimeMs = 0;

    AACPriority m_lastPriority = AACPriority::VeryLow;
    qint64 m_lastPriorityTimeMs = 0;
};
