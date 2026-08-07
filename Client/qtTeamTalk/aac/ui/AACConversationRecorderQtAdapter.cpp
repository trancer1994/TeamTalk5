#include "AACConversationRecorderQtAdapter.h"
#include <QtMath>
#include <algorithm>

AACConversationRecorderQtAdapter::AACConversationRecorderQtAdapter(
        AACAccessibilityManager* aac,
        QObject* parent)
    : QObject(parent)
    , m_aac(aac)
    , m_backend(aac, this)
{
    // If backend has its own signal, you can forward it, but we’ll
    // emit conversationUpdated from this adapter whenever we change m_events.
}

AACConversationRecorderQtAdapter::Event AACConversationRecorderQtAdapter::makeEvent(
        EventType type,
        const QString& channelId,
        const QString& userId,
        const QString& text,
        const QString& semanticTag,
        bool emergency) const
{
    Event ev;
    ev.type        = type;
    ev.timestamp   = QDateTime::currentDateTimeUtc();
    ev.userId      = userId;
    ev.channelId   = channelId;
    ev.text        = text;
    ev.semanticTag = semanticTag;
    ev.emergency   = emergency;
    return ev;
}

void AACConversationRecorderQtAdapter::appendEvent(const Event& ev)
{
    m_events << ev;
    rebuildPcmStream();
    emit conversationUpdated(m_events);
}

// --- Recording API ---

void AACConversationRecorderQtAdapter::recordAACMessageSent(
        const AACMessage& msg,
        const QString& channelId,
        const QString& userId,
        const QString& semanticTag,
        bool emergency)
{
    // Backend semantic recording
    m_backend.recordAACMessageSent(msg, channelId, userId, semanticTag, emergency);

    // Qt-facing event
    Event ev = makeEvent(EventType::AACMessageSent,
                         channelId,
                         userId,
                         msg.text,
                         semanticTag,
                         emergency);
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordAACMessageSpoken(
        const AACMessage& msg,
        const QString& channelId,
        const QString& userId,
        const QString& semanticTag,
        bool emergency)
{
    m_backend.recordAACMessageSpoken(msg, channelId, userId, semanticTag, emergency);

    Event ev = makeEvent(EventType::AACMessageSpoken,
                         channelId,
                         userId,
                         msg.text,
                         semanticTag,
                         emergency);
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordAudioFromUser(
        const QString& userId,
        const QString& channelId)
{
    m_backend.recordAudioFromUser(userId, channelId);

    Event ev = makeEvent(EventType::AudioFromUser, channelId, userId);
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordAudioToUser(
        const QString& userId,
        const QString& channelId)
{
    m_backend.recordAudioToUser(userId, channelId);

    Event ev = makeEvent(EventType::AudioToUser, channelId, userId);
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordAudioFrame(
        const QString& userId,
        const QString& channelId,
        const QByteArray& pcm,
        int sampleRate,
        int channels)
{
    m_backend.recordAudioFrame(userId, channelId, pcm, sampleRate, channels);

    Event ev = makeEvent(EventType::AudioFromUser, channelId, userId);
    ev.audioPcm        = pcm;
    ev.audioSampleRate = sampleRate;
    ev.audioChannels   = channels;
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordChannelJoin(const QString& channelId)
{
    m_backend.recordChannelJoin(channelId);

    Event ev = makeEvent(EventType::ChannelJoin, channelId);
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordChannelLeave(const QString& channelId)
{
    m_backend.recordChannelLeave(channelId);

    Event ev = makeEvent(EventType::ChannelLeave, channelId);
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordTransmitOn(const QString& channelId)
{
    m_backend.recordTransmitOn(channelId);

    Event ev = makeEvent(EventType::TransmitOn, channelId);
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordTransmitOff(const QString& channelId)
{
    m_backend.recordTransmitOff(channelId);

    Event ev = makeEvent(EventType::TransmitOff, channelId);
    appendEvent(ev);
}

void AACConversationRecorderQtAdapter::recordSystemNotification(
        const QString& text,
        const QString& channelId)
{
    m_backend.recordSystemNotification(text, channelId);

    Event ev = makeEvent(EventType::SystemNotification, channelId, QString(), text, "system", false);
    appendEvent(ev);
}

// --- Search / filter / jump ---

QList<int> AACConversationRecorderQtAdapter::searchText(const QString& query) const
{
    QList<int> out;
    if (query.isEmpty())
        return out;

    for (int i = 0; i < m_events.size(); ++i) {
        if (m_events[i].text.contains(query, Qt::CaseInsensitive))
            out << i;
    }
    return out;
}

QList<int> AACConversationRecorderQtAdapter::filterByTag(const QString& tag) const
{
    QList<int> out;
    for (int i = 0; i < m_events.size(); ++i) {
        if (m_events[i].semanticTag == tag)
            out << i;
    }
    return out;
}

QList<int> AACConversationRecorderQtAdapter::filterByType(EventType type) const
{
    QList<int> out;
    for (int i = 0; i < m_events.size(); ++i) {
        if (m_events[i].type == type)
            out << i;
    }
    return out;
}

QList<int> AACConversationRecorderQtAdapter::filterByDirection(bool toUser) const
{
    QList<int> out;
    for (int i = 0; i < m_events.size(); ++i) {
        const auto& ev = m_events[i];
        if (toUser && ev.type == EventType::AudioToUser)
            out << i;
        else if (!toUser && ev.type == EventType::AudioFromUser)
            out << i;
    }
    return out;
}

int AACConversationRecorderQtAdapter::nextEventWithTag(const QString& tag, int fromIndex) const
{
    for (int i = qMax(0, fromIndex); i < m_events.size(); ++i) {
        if (m_events[i].semanticTag == tag)
            return i;
    }
    return -1;
}

int AACConversationRecorderQtAdapter::nextEmergencyEvent(int fromIndex) const
{
    for (int i = qMax(0, fromIndex); i < m_events.size(); ++i) {
        if (m_events[i].emergency)
            return i;
    }
    return -1;
}

int AACConversationRecorderQtAdapter::nextAACMessage(int fromIndex) const
{
    for (int i = qMax(0, fromIndex); i < m_events.size(); ++i) {
        if (m_events[i].type == EventType::AACMessageSent ||
            m_events[i].type == EventType::AACMessageSpoken)
            return i;
    }
    return -1;
}

int AACConversationRecorderQtAdapter::nextAudioEvent(int fromIndex) const
{
    for (int i = qMax(0, fromIndex); i < m_events.size(); ++i) {
        if (m_events[i].type == EventType::AudioFromUser ||
            m_events[i].type == EventType::AudioToUser)
            return i;
    }
    return -1;
}

int AACConversationRecorderQtAdapter::firstEventOfType(EventType type) const
{
    for (int i = 0; i < m_events.size(); ++i) {
        if (m_events[i].type == type)
            return i;
    }
    return -1;
}

int AACConversationRecorderQtAdapter::lastEventOfType(EventType type) const
{
    for (int i = m_events.size() - 1; i >= 0; --i) {
        if (m_events[i].type == type)
            return i;
    }
    return -1;
}

// --- Bookmarks ---

bool AACConversationRecorderQtAdapter::isBookmarked(int index) const
{
    if (index < 0 || index >= m_events.size())
        return false;
    return m_events[index].bookmarked;
}

void AACConversationRecorderQtAdapter::addBookmark(int index, const QString& note)
{
    if (index < 0 || index >= m_events.size())
        return;
    m_events[index].bookmarked   = true;
    m_events[index].bookmarkNote = note;
    emit conversationUpdated(m_events);
}

void AACConversationRecorderQtAdapter::removeBookmark(int index)
{
    if (index < 0 || index >= m_events.size())
        return;
    m_events[index].bookmarked   = false;
    m_events[index].bookmarkNote = QString();
    emit conversationUpdated(m_events);
}

QList<int> AACConversationRecorderQtAdapter::bookmarkedEvents() const
{
    QList<int> out;
    for (int i = 0; i < m_events.size(); ++i) {
        if (m_events[i].bookmarked)
            out << i;
    }
    return out;
}

// --- PCM stream rebuild ---

void AACConversationRecorderQtAdapter::rebuildPcmStream()
{
    m_pcmStream.clear();
    m_pcmSampleRate = 48000;
    m_pcmChannels   = 1;

    for (const auto& ev : m_events) {
        if (!ev.audioPcm.isEmpty()) {
            m_pcmStream.append(ev.audioPcm);
            m_pcmSampleRate = ev.audioSampleRate;
            m_pcmChannels   = ev.audioChannels;
        }
    }
}

// --- WAV export ---

QByteArray AACConversationRecorderQtAdapter::exportConversationAudioWav() const
{
    if (m_pcmStream.isEmpty() || m_pcmSampleRate <= 0 || m_pcmChannels <= 0)
        return QByteArray();

    QByteArray out;
    out.reserve(m_pcmStream.size() + 44);

    auto writeLE32 = [&](quint32 v) {
        out.append(char(v & 0xFF));
        out.append(char((v >> 8) & 0xFF));
        out.append(char((v >> 16) & 0xFF));
        out.append(char((v >> 24) & 0xFF));
    };

    auto writeLE16 = [&](quint16 v) {
        out.append(char(v & 0xFF));
        out.append(char((v >> 8) & 0xFF));
    };

    // RIFF header
    out.append("RIFF", 4);
    writeLE32(36 + m_pcmStream.size()); // file size - 8
    out.append("WAVE", 4);

    // fmt chunk
    out.append("fmt ", 4);
    writeLE32(16);                      // PCM
    writeLE16(1);                       // format tag = PCM
    writeLE16(quint16(m_pcmChannels));
    writeLE32(quint32(m_pcmSampleRate));
    quint16 bitsPerSample = 16;
    quint32 byteRate      = m_pcmSampleRate * m_pcmChannels * bitsPerSample / 8;
    quint16 blockAlign    = m_pcmChannels * bitsPerSample / 8;
    writeLE32(byteRate);
    writeLE16(blockAlign);
    writeLE16(bitsPerSample);

    // data chunk
    out.append("data", 4);
    writeLE32(m_pcmStream.size());
    out.append(m_pcmStream);

    return out;
}

// --- Playback ---

void AACConversationRecorderQtAdapter::playConversationAudio()
{
    bool fatigue = m_aac && m_aac->modes().fatigueMode;
    m_audio.playPcm(m_pcmStream, m_pcmSampleRate, m_pcmChannels, fatigue);
}

void AACConversationRecorderQtAdapter::playEventAudio(int index)
{
    if (index < 0 || index >= m_events.size())
        return;

    const auto& ev = m_events[index];
    if (ev.audioPcm.isEmpty() || ev.audioSampleRate <= 0 || ev.audioChannels <= 0)
        return;

    bool fatigue = m_aac && m_aac->modes().fatigueMode;
    m_audio.playPcm(ev.audioPcm, ev.audioSampleRate, ev.audioChannels, fatigue);
}

void AACConversationRecorderQtAdapter::playLastAACMessage()
{
    int idx = lastEventOfType(EventType::AACMessageSent);
    if (idx < 0)
        idx = lastEventOfType(EventType::AACMessageSpoken);
    if (idx >= 0)
        playEventAudio(idx);
}

void AACConversationRecorderQtAdapter::playLastSpokenMessage()
{
    int idx = lastEventOfType(EventType::AACMessageSpoken);
    if (idx >= 0)
        playEventAudio(idx);
}

void AACConversationRecorderQtAdapter::playLastAudioFrame()
{
    int idx = lastEventOfType(EventType::AudioFromUser);
    if (idx < 0)
        idx = lastEventOfType(EventType::AudioToUser);
    if (idx >= 0)
        playEventAudio(idx);
}

void AACConversationRecorderQtAdapter::stopConversationAudio()
{
    m_audio.stop();
}

void AACConversationRecorderQtAdapter::pauseConversationAudio()
{
    m_audio.pause();
}

void AACConversationRecorderQtAdapter::resumeConversationAudio()
{
    m_audio.resume();
}

// --- Text export ---

QString AACConversationRecorderQtAdapter::exportToText() const
{
    QString out;
    for (const auto& ev : m_events) {
        out += QString("[%1] %2\n")
            .arg(ev.timestamp.toString("hh:mm:ss"))
            .arg(ev.text.isEmpty() ? QObject::tr("(no text)") : ev.text);
    }
    return out;
}
