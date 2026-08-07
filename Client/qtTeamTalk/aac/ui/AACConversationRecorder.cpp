#include "AACConversationRecorder.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using Clock = std::chrono::system_clock;

// ---------------------------------------------------------------------
// Timestamp helper
// ---------------------------------------------------------------------

static std::string timestampNow()
{
    auto now = Clock::now();
    std::time_t t = Clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

// ---------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------

AACConversationRecorder::AACConversationRecorder()
{
}

// ---------------------------------------------------------------------
// Internal helper: build event
// ---------------------------------------------------------------------

AACConversationRecorder::Event
AACConversationRecorder::makeEvent(EventType type,
                                   const std::string& text,
                                   const std::string& semanticTag,
                                   const std::string& role,
                                   bool emergency,
                                   const std::string& channelId,
                                   const std::string& userId) const
{
    Event ev;
    ev.timestamp   = timestampNow();
    ev.type        = type;
    ev.text        = text;
    ev.semanticTag = semanticTag;
    ev.role        = role;
    ev.emergency   = emergency;
    ev.channelId   = channelId;
    ev.userId      = userId;
    return ev;
}

// ---------------------------------------------------------------------
// API‑layer semantic events
// ---------------------------------------------------------------------

void AACConversationRecorder::onMessageSent(const std::string& text,
                                            const std::string& semanticTag,
                                            const std::string& role,
                                            bool emergency,
                                            const std::string& channelId,
                                            const std::string& userId)
{
    m_events.push_back(makeEvent(EventType::AACMessageSent,
                                 text,
                                 semanticTag,
                                 role,
                                 emergency,
                                 channelId,
                                 userId));
}

void AACConversationRecorder::onMessageSpoken(const std::string& text,
                                              const std::string& semanticTag,
                                              const std::string& role,
                                              bool emergency,
                                              const std::string& channelId,
                                              const std::string& userId)
{
    m_events.push_back(makeEvent(EventType::AACMessageSpoken,
                                 text,
                                 semanticTag,
                                 role,
                                 emergency,
                                 channelId,
                                 userId));
}

void AACConversationRecorder::onAudioFromUser(const std::string& userId,
                                              const std::string& channelId)
{
    m_events.push_back(makeEvent(EventType::AudioFromUser,
                                 "",
                                 "",
                                 "",
                                 false,
                                 channelId,
                                 userId));
}

void AACConversationRecorder::onAudioToUser(const std::string& userId,
                                            const std::string& channelId)
{
    m_events.push_back(makeEvent(EventType::AudioToUser,
                                 "",
                                 "",
                                 "",
                                 false,
                                 channelId,
                                 userId));
}

void AACConversationRecorder::onChannelJoin(const std::string& channelId)
{
    m_events.push_back(makeEvent(EventType::ChannelJoin,
                                 "",
                                 "",
                                 "",
                                 false,
                                 channelId,
                                 ""));
}

void AACConversationRecorder::onChannelLeave(const std::string& channelId)
{
    m_events.push_back(makeEvent(EventType::ChannelLeave,
                                 "",
                                 "",
                                 "",
                                 false,
                                 channelId,
                                 ""));
}

void AACConversationRecorder::onTransmitOn(const std::string& channelId)
{
    m_events.push_back(makeEvent(EventType::TransmitOn,
                                 "",
                                 "",
                                 "",
                                 false,
                                 channelId,
                                 ""));
}

void AACConversationRecorder::onTransmitOff(const std::string& channelId)
{
    m_events.push_back(makeEvent(EventType::TransmitOff,
                                 "",
                                 "",
                                 "",
                                 false,
                                 channelId,
                                 ""));
}

void AACConversationRecorder::onSystemNotification(const std::string& text,
                                                   const std::string& channelId)
{
    m_events.push_back(makeEvent(EventType::SystemNotification,
                                 text,
                                 "",
                                 "",
                                 false,
                                 channelId,
                                 ""));
}

// ---------------------------------------------------------------------
// Export
// ---------------------------------------------------------------------

std::string AACConversationRecorder::exportToText() const
{
    std::ostringstream out;

    for (const auto& ev : m_events) {
        out << ev.timestamp << " | ";

        switch (ev.type) {
        case EventType::AACMessageSent:      out << "AAC_SENT"; break;
        case EventType::AACMessageSpoken:    out << "AAC_SPOKEN"; break;
        case EventType::AudioFromUser:       out << "AUDIO_FROM_USER"; break;
        case EventType::AudioToUser:         out << "AUDIO_TO_USER"; break;
        case EventType::ChannelJoin:         out << "CHANNEL_JOIN"; break;
        case EventType::ChannelLeave:        out << "CHANNEL_LEAVE"; break;
        case EventType::TransmitOn:          out << "TRANSMIT_ON"; break;
        case EventType::TransmitOff:         out << "TRANSMIT_OFF"; break;
        case EventType::SystemNotification:  out << "SYSTEM"; break;
        }

        if (!ev.channelId.empty())
            out << " | ch=" << ev.channelId;
        if (!ev.userId.empty())
            out << " | user=" << ev.userId;
        if (!ev.semanticTag.empty())
            out << " | tag=" << ev.semanticTag;
        if (!ev.role.empty())
            out << " | role=" << ev.role;
        if (ev.emergency)
            out << " | EMERGENCY";

        if (!ev.text.empty())
            out << " | \"" << ev.text << "\"";

        out << "\n";
    }

    return out.str();
}
