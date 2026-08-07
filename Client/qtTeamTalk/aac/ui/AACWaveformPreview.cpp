#include "AACWaveformPreview.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

AACWaveformPreview::AACWaveformPreview(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(80);
    setAccessibleName(tr("Waveform preview"));
}

void AACWaveformPreview::setSamples(const QVector<float>& samples)
{
    m_samples = samples;
    update();
}

void AACWaveformPreview::setForegroundColor(const QColor& color)
{
    m_foreground = color;
    update();
}

void AACWaveformPreview::setBackgroundColor(const QColor& color)
{
    m_background = color;
    update();
}

void AACWaveformPreview::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.fillRect(rect(), m_background);

    if (m_samples.isEmpty())
        return;

    int w = width();
    int h = height();
    int n = m_samples.size();

    p.setPen(QPen(m_foreground, 1));

    for (int x = 0; x < w; ++x) {
        int idx = (x * n) / w;
        float value = std::clamp(m_samples[idx], -1.0f, 1.0f);
        int y = h / 2;
        int amplitude = static_cast<int>((h / 2) * value);
        p.drawLine(x, y - amplitude, x, y + amplitude);
    }
setAccessibleDescription(tr("Waveform preview showing %1 samples").arg(m_samples.size()));
}
