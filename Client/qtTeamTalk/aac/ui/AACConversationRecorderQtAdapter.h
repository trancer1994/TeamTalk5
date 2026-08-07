#pragma once

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QString>
#include <QByteArray>
#include <QList>

#include "AACConversationRecorder.h"
#include "AACAudioEngine.h"
#include "AACAccessibilityManager.h"

class AACConversationRecorderQtAdapter : public QObject
{
    Q_OBJECT

public:
    using EventType = AACConversationRecorder::EventType;

    struct Event {
        EventType type;
        QDateTime timestamp;

        QString userId;
        QString channelId;
        QString text;
        QString semanticTag;
        bool    emergency = false;

        bool    bookmarked = false;
        QString bookmarkNote;

        QByteArray audioPcm;
        int        audioSampleRate = 0;
        int        audioChannels   = 0;
    };

    explicit AACConversationRecorderQtAdapter(AACAccessibilityManager* aac,
                                              QObject* parent = nullptr);

    // --- Recording API (Qt-facing, emulates old recorder) ---
    void recordAACMessageSent(const AACMessage& msg,
                              const QString& channelId,
                              const QString& userId,
                              const QString& semanticTag,
                              bool emergency);

    void recordAACMessageSpoken(const AACMessage& msg,
                                const QString& channelId,
                                const QString& userId,
                                const QString& semanticTag,
                                bool emergency);

    void recordAudioFromUser(const QString& userId,
                             const QString& channelId);

    void recordAudioToUser(const QString& userId,
                           const QString& channelId);

    void recordAudioFrame(const QString& userId,
                          const QString& channelId,
                          const QByteArray& pcm,
                          int sampleRate,
                          int channels);

    void recordChannelJoin(const QString& channelId);
    void recordChannelLeave(const QString& channelId);

    void recordTransmitOn(const QString& channelId);
    void recordTransmitOff(const QString& channelId);

    void recordSystemNotification(const QString& text,
                                  const QString& channelId = QString());

    // --- Event access ---
    const QVector<Event>& events() const { return m_events; }
    bool isEmpty() const { return m_events.isEmpty(); }

    // --- Search / filter / jump ---
    QList<int> searchText(const QString& query) const;
    QList<int> filterByTag(const QString& tag) const;
    QList<int> filterByType(EventType type) const;
    QList<int> filterByDirection(bool toUser) const;

    int nextEventWithTag(const QString& tag, int fromIndex) const;
    int nextEmergencyEvent(int fromIndex) const;
    int nextAACMessage(int fromIndex) const;
    int nextAudioEvent(int fromIndex) const;

    int firstEventOfType(EventType type) const;
    int lastEventOfType(EventType type) const;

    // --- Bookmarks ---
    bool isBookmarked(int index) const;
    void addBookmark(int index, const QString& note = QString());
    void removeBookmark(int index);
    QList<int> bookmarkedEvents() const;

    // --- Audio export / playback ---
    QByteArray exportConversationAudioWav() const;

    void playConversationAudio();
    void playEventAudio(int index);
    void playLastAACMessage();
    void playLastSpokenMessage();
    void playLastAudioFrame();
    void stopConversationAudio();
    void pauseConversationAudio();
    void resumeConversationAudio();

    QString exportToText() const;

signals:
    void conversationUpdated(const QVector<Event>& events);

private:
    AACAccessibilityManager* m_aac = nullptr;
    AACConversationRecorder   m_backend;   // AAC-idiomatic backend
    AACAudioEngine            m_audio;     // Qt audio engine

    QVector<Event> m_events;

    QByteArray m_pcmStream;
    int        m_pcmSampleRate = 48000;
    int        m_pcmChannels   = 1;

    Event makeEvent(EventType type,
                    const QString& channelId,
                    const QString& userId = QString(),
                    const QString& text = QString(),
                    const QString& semanticTag = QString(),
                    bool emergency = false) const;

    void appendEvent(const Event& ev);
    void rebuildPcmStream();
};
