#pragma once

#include <QWidget>
#include <QVector>
#include <QColor>

class AACWaveformPreview : public QWidget
{
    Q_OBJECT

public:
    explicit AACWaveformPreview(QWidget* parent = nullptr);

    void setSamples(const QVector<float>& samples);
    void setForegroundColor(const QColor& color);
    void setBackgroundColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<float> m_samples;
    QColor m_foreground = Qt::blue;
    QColor m_background = Qt::black;
};
