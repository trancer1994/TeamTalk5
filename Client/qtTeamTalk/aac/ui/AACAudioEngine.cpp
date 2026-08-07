#include "AACAudioEngine.h"
#include <QAudioFormat>

AACAudioEngine::AACAudioEngine(QObject* parent)
    : QObject(parent)
{
}

void AACAudioEngine::playPcm(const QByteArray& pcm, int sampleRate, int channels, bool fatigueMode)
{
    stop(); // clean start

    m_buffer = new QBuffer(this);
    m_buffer->setData(pcm);
    m_buffer->open(QIODevice::ReadOnly);

    QAudioFormat fmt;
    fmt.setSampleRate(sampleRate);
    fmt.setChannelCount(channels);
    fmt.setSampleFormat(QAudioFormat::Int16);

    m_output = new QAudioOutput(fmt, this);

    // Fatigue‑mode shaping (simple, AAC‑friendly)
    const qreal normalVolume  = 0.9;
    const qreal fatigueVolume = 0.55;
    m_output->setVolume(fatigueMode ? fatigueVolume : normalVolume);

    m_output->start(m_buffer);
}

void AACAudioEngine::stop()
{
    if (m_output) {
        m_output->stop();
        m_output->deleteLater();
        m_output = nullptr;
    }

    if (m_buffer) {
        m_buffer->close();
        m_buffer->deleteLater();
        m_buffer = nullptr;
    }
}

void AACAudioEngine::pause()
{
    if (m_output)
        m_output->suspend();
}

void AACAudioEngine::resume()
{
    if (m_output)
        m_output->resume();
}
