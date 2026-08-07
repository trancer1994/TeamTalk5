#pragma once
#include <QObject>
#include <QAudioOutput>
#include <QBuffer>

class AACAudioEngine : public QObject
{
    Q_OBJECT

public:
    explicit AACAudioEngine(QObject* parent = nullptr);

    void playPcm(const QByteArray& pcm, int sampleRate, int channels, bool fatigueMode);
    void stop();
    void pause();
    void resume();

private:
    QAudioOutput* m_output = nullptr;
    QBuffer*      m_buffer = nullptr;
};
