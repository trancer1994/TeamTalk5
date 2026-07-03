#include "AACKeyButton.h"

#include <QPainter>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QFocusEvent>

AACKeyButton::AACKeyButton(const QString& text,
                           AACAccessibilityManager* aac,
                           QWidget* parent)
    : QPushButton(text, parent)
    , m_aac(aac)
    , m_keyText(text)
{
    setFocusPolicy(Qt::StrongFocus);

    // --- Long‑press timer ---
    m_longPressTimer = new QTimer(this);
    m_longPressTimer->setSingleShot(true);
m_longPressTimer->setInterval(m_deepWell ? 650 : 500);
    connect(m_longPressTimer, &QTimer::timeout, this, [this]() {
        m_isLongPressActive = true;
        emit longPressed();
    });

    // --- Dwell progress subscription ---
    if (m_aac) {
        connect(m_aac->inputController(),
                &AACInputController::dwellProgressChanged,
                this,
                [this](QWidget* target, float p) {
                    if (target == this)
                        setDwellProgress(p);
                });
    }
}

void AACKeyButton::setHighlighted(bool on)
{
    m_highlighted = on;
    update();
}

void AACKeyButton::setDeepWell(bool enabled)
{
    m_deepWell = enabled;
    setProperty("aacDeepWell", enabled);
}
void AACKeyButton::setDwellProgress(float p)
{
    m_dwellProgress = qBound(0.0f, p, 1.0f);
    update();
}

void AACKeyButton::enterEvent(QEnterEvent* event)
{
    QPushButton::enterEvent(event);

    emit hovered(this);

    // --- Dwell activation (curated strip or any AACKeyButton) ---
    if (m_aac &&
        m_aac->modes().dwell &&
        m_aac->modes().curatedStripDwell)
    {
        m_aac->inputController()->startDwellOn(this);
    }
}

void AACKeyButton::mousePressEvent(QMouseEvent* e)
{
    const bool fatigue = m_aac && m_aac->modes().fatigueMode;

    // --- Fatigue‑mode double‑activation guard ---
    static QElapsedTimer s_fatigueGuard;
    if (!s_fatigueGuard.isValid())
        s_fatigueGuard.start();

    if (fatigue) {
        // Ignore rapid accidental re‑activations (<600ms)
        if (s_fatigueGuard.elapsed() < 600) {
            e->accept();
            return;
        }
        s_fatigueGuard.restart();
    }

    // --- Sensory suppression (no click sound in fatigue mode) ---
    if (!fatigue && m_aac && m_aac->feedbackEngine()) {
        m_aac->feedbackEngine()->playClick();
    }

    // --- Your existing long‑press logic ---
    m_isLongPressActive = false;
    m_longPressTimer->start();

    QPushButton::mousePressEvent(e);
}

void AACKeyButton::mouseReleaseEvent(QMouseEvent* e)
{
    m_longPressTimer->stop();

    // Only activate if the pointer is still inside the button
    if (!rect().contains(e->pos())) {
        m_isLongPressActive = false;
        QPushButton::mouseReleaseEvent(e);
        return;
    }

    const bool fatigue = m_aac && m_aac->modes().fatigueMode;

    // --- Fatigue‑mode double‑activation guard ---
    static QElapsedTimer s_fatigueGuard;
    if (!s_fatigueGuard.isValid())
        s_fatigueGuard.start();

    if (fatigue) {
        // Ignore rapid accidental re‑activations (<600ms)
        if (s_fatigueGuard.elapsed() < 600) {
            e->accept();
            m_isLongPressActive = false;
            return;
        }
        s_fatigueGuard.restart();
    }

if (m_deepWell) {
    if (m_aac && m_aac->feedbackEngine()) {
        m_aac->feedbackEngine()->playDeepWellHaptic(m_aac->modes().fatigueMode);
    }
    emit deepWellActivated(m_keyText);
}

    // --- Normal activation (only if not long‑press) ---
    if (!m_isLongPressActive) {
        emit keyActivated(m_keyText);
    }

    m_isLongPressActive = false;
    QPushButton::mouseReleaseEvent(e);
}

void AACKeyButton::paintEvent(QPaintEvent* event)
{
    QPushButton::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRect r = rect().adjusted(2, 2, -2, -2);

    // --- 1. Scanning highlight (amber) ---
    if (m_highlighted) {
        p.save();
        QPen pen(QColor(255, 193, 7)); // amber
        pen.setWidth(3);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r, 6, 6);
        p.restore();
    }

    // --- 2. Semantic highlight (blue) ---
    if (m_semanticHighlighted) {
        p.save();
        QPen pen(QColor(0, 120, 215)); // Windows blue
        pen.setWidth(3);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(3, 3, -3, -3), 6, 6);
        p.restore();
    }

    // --- 3. Dwell progress arc ---
    if (m_dwellProgress > 0.0f &&
        m_aac &&
        m_aac->modes().dwell &&
        m_aac->modes().curatedStripDwell)
    {
        p.save();
        QPen pen(QColor(0, 120, 215)); // blue
        pen.setWidth(3);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        const int startAngle = 90 * 16;
        const int spanAngle  = -static_cast<int>(360 * 16 * m_dwellProgress);
        QRect arcRect = r.adjusted(3, 3, -3, -3);
        p.drawArc(arcRect, startAngle, spanAngle);
        p.restore();
    }

    // --- 4. Deep‑well accent (red) ---
    if (m_deepWell) {
        p.save();
        QPen pen(QColor(200, 50, 50)); // subtle red accent
        pen.setWidth(4);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(1, 1, -1, -1), 6, 6);
        p.restore();
    }
}
