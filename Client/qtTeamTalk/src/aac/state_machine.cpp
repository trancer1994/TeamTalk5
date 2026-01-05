#include "state_machine.h"
#include "events.h"

namespace AAC {

void StateMachine::handleEvent(const Event& event)
{
    switch (event.type) {

    case EventType::ConnectRequested:
        if (m_state == State::Idle) {
            m_state = State::Connecting;
        }
        break;

    case EventType::Connected:
        if (m_state == State::Connecting) {
            m_state = State::Connected;
        }
        break;

    case EventType::ConnectionFailed:
        if (m_state == State::Connecting) {
            m_state = State::Error;
        }
        break;

    case EventType::Disconnected:
        if (m_state == State::Connected || m_state == State::Error) {
            m_state = State::Idle;
        }
        break;

    case EventType::None:
    default:
        break;
    }
}

} // namespace AAC
