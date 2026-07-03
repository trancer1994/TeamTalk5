#include "AACEarconRouter.h"

AACEarconRouter::AACEarconRouter(AACFramework* aac, QObject* parent)
    : QObject(parent),
      m_aac(aac)
{
    m_timer.start();
}

bool AACEarconRouter::shouldSuppress(AACEvent ev, int minIntervalMs)
{
    qint64 now = m_timer.elapsed();

    if (ev == m_lastEvent && (now - m_lastTimeMs) < minIntervalMs)
        return true;

    m_lastEvent = ev;
    m_lastTimeMs = now;
    return false;
}

bool AACEarconRouter::priorityAllows(AACPriority p)
{
    qint64 now = m_timer.elapsed();

    // Higher priority → always allow
    if (p < m_lastPriority) {
        m_lastPriority = p;
        m_lastPriorityTimeMs = now;
        return true;
    }

    // Same priority → allow if 120ms passed
    if (p == m_lastPriority && (now - m_lastPriorityTimeMs) > 120) {
        m_lastPriorityTimeMs = now;
        return true;
    }

    // Lower priority → suppress
    return false;
}

void AACEarconRouter::play(AACEvent ev, AACPriority p)
{
    // Help must always play immediately
    if (ev == AACEvent::Help) {
        m_aac->feedback(ev);
        m_lastPriority = p;
        m_lastPriorityTimeMs = m_timer.elapsed();
        return;
    }

    if (!priorityAllows(p))
        return;

    m_aac->feedback(ev);
}

// -------------------------
// Navigation
// -------------------------

void AACEarconRouter::navForward()
{
    if (shouldSuppress(AACEvent::NavigateForward, 150))
        return;
    play(AACEvent::NavigateForward, AACPriority::Nav);
}

void AACEarconRouter::navBack()
{
    if (shouldSuppress(AACEvent::NavigateBack, 150))
        return;
    play(AACEvent::NavigateBack, AACPriority::Nav);
}

// -------------------------
// Focus
// -------------------------

void AACEarconRouter::focusInit()
{
    if (shouldSuppress(AACEvent::FocusInitialised, 80))
        return;
    play(AACEvent::FocusInitialised, AACPriority::VeryLow);
}

// -------------------------
// Activation
// -------------------------

void AACEarconRouter::activate()
{
    if (shouldSuppress(AACEvent::Activate, 60))
        return;
    play(AACEvent::Activate, AACPriority::Low);
}

// -------------------------
// Confirmation
// -------------------------

void AACEarconRouter::confirm()
{
    if (shouldSuppress(AACEvent::Confirm, 120))
        return;
    play(AACEvent::Confirm, AACPriority::Medium);
}

// -------------------------
// Errors
// -------------------------

void AACEarconRouter::error()
{
    if (shouldSuppress(AACEvent::Error, 300))
        return;
    play(AACEvent::Error, AACPriority::High);
}

// -------------------------
// Messaging
// -------------------------

void AACEarconRouter::messageReceived()
{
    if (shouldSuppress(AACEvent::MessageReceived, 150))
        return;
    play(AACEvent::MessageReceived, AACPriority::VeryLow);
}

void AACEarconRouter::messageSent()
{
    if (shouldSuppress(AACEvent::MessageSent, 120))
        return;
    play(AACEvent::MessageSent, AACPriority::Low);
}

// -------------------------
// Channel events
// -------------------------

void AACEarconRouter::channelJoin()
{
    if (shouldSuppress(AACEvent::ChannelJoin, 200))
        return;
    play(AACEvent::ChannelJoin, AACPriority::VeryLow);
}

void AACEarconRouter::channelLeave()
{
    if (shouldSuppress(AACEvent::ChannelLeave, 200))
        return;
    play(AACEvent::ChannelLeave, AACPriority::VeryLow);
}

void AACEarconRouter::userJoin()
{
    if (shouldSuppress(AACEvent::UserJoin, 200))
        return;
    play(AACEvent::UserJoin, AACPriority::VeryLow);
}

void AACEarconRouter::userLeave()
{
    if (shouldSuppress(AACEvent::UserLeave, 200))
        return;
    play(AACEvent::UserLeave, AACPriority::VeryLow);
}

// -------------------------
// Reconnect events
// -------------------------

void AACEarconRouter::reconnectAttempt()
{
    if (shouldSuppress(AACEvent::ReconnectAttempt, 500))
        return;
    play(AACEvent::ReconnectAttempt, AACPriority::Nav);
}

void AACEarconRouter::reconnectFailed()
{
    if (shouldSuppress(AACEvent::ReconnectFailed, 500))
        return;
    play(AACEvent::ReconnectFailed, AACPriority::Nav);
}

void AACEarconRouter::reconnectSuccess()
{
    if (shouldSuppress(AACEvent::ReconnectSuccess, 500))
        return;
    play(AACEvent::ReconnectSuccess, AACPriority::Nav);
}

// -------------------------
// Help
// -------------------------

void AACEarconRouter::help()
{
    play(AACEvent::Help, AACPriority::Help);
}
