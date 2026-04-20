#include "AACKeyButton.h"
#include <QPainter>
#include <QStyleOption>

AACKeyButton::AACKeyButton(const QString& text,
                           AACAccessibilityManager* aac,
                           QWidget* parent)
    : QPushButton(text, parent)
    , m_aac(aac)
    , m_keyText(text)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(60, 60);

    setMouseTracking(true);

    connect(this, &QPushButton::clicked,
            this, &AACKeyButton::handleClick);
}

void AACKeyButton::setHighlighted(bool on)
{
    if (m_highlighted == on)
        return;

    m_highlighted = on;
    update();
}

void AACKeyButton::enterEvent(QEnterEvent* event)
{
    emit hovered(this);
    QPushButton::enterEvent(event);
}

void AACKeyButton::paintEvent(QPaintEvent* event)
{
    QStyleOptionButton opt;
    initStyleOption(&opt);

    QPainter p(this);

    // Background
    if (m_highlighted) {
        p.fillRect(rect(), QColor(0, 80, 200));   // deep blue
        p.setPen(Qt::white);
    } else {
        p.fillRect(rect(), QColor(240, 240, 240)); // light grey
        p.setPen(Qt::black);
    }

    // Border
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(m_highlighted ? QColor(0, 40, 120) : QColor(180, 180, 180), m_highlighted ? 3 : 1));
    p.drawRect(rect().adjusted(1, 1, -2, -2));

    // Text
    p.drawText(rect(), Qt::AlignCenter, m_keyText);
}

void AACKeyButton::handleClick()
{
    emit keyActivated(m_keyText);

    if (m_aac)
        m_aac->inputController()->insertText(m_keyText);
}
