#pragma once

#include <string>
#include <vector>

class AACConversationRecorder
{
public:
    enum class EventType {
        AACMessageSent,
        AACMessageSpoken,
        AudioFromUser,
        AudioToUser,
        ChannelJoin,
        ChannelLeave,
        TransmitOn,
        TransmitOff,
        SystemNotification
    };

    struct Event {
        std::string timestamp;    // ISO‑8601
        EventType   type;

        std::string userId;       // optional: speaker / target
        std::string channelId;    // optional: channel context
        std::string text;         // AAC text or notification text
        std::string semanticTag;  // e.g. "need_help", "emotion_happy"
        std::string role;         // e.g. "need", "emotion", "social"
        bool        emergency = false;
    };

    AACConversationRecorder();

    // --- API‑layer semantic events ---
    void onMessageSent(const std::string& text,
                       const std::string& semanticTag,
                       const std::string& role,
                       bool emergency,
                       const std::string& channelId,
                       const std::string& userId);

    void onMessageSpoken(const std::string& text,
                         const std::string& semanticTag,
                         const std::string& role,
                         bool emergency,
                         const std::string& channelId,
                         const std::string& userId);

    void onAudioFromUser(const std::string& userId,
                         const std::string& channelId);

    void onAudioToUser(const std::string& userId,
                       const std::string& channelId);

    void onChannelJoin(const std::string& channelId);
    void onChannelLeave(const std::string& channelId);

    void onTransmitOn(const std::string& channelId);
    void onTransmitOff(const std::string& channelId);

    void onSystemNotification(const std::string& text,
                              const std::string& channelId);

    // --- Accessors / export ---
    const std::vector<Event>& events() const { return m_events; }
    bool isEmpty() const { return m_events.empty(); }

    std::string exportToText() const;

private:
    std::vector<Event> m_events;

    Event makeEvent(EventType type,
                    const std::string& text,
                    const std::string& semanticTag,
                    const std::string& role,
                    bool emergency,
                    const std::string& channelId,
                    const std::string& userId) const;
};