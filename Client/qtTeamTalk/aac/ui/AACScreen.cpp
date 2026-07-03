#include "AACScreen.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

AACScreen::AACScreen(AACAccessibilityManager* aac, QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
{
}

void AACScreen::applyVisualPulse(int strength)
{
    int duration = (strength == 1 ? 80 : strength == 2 ? 120 : 160);

    auto* eff = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(eff);

    auto* anim = new QPropertyAnimation(eff, "opacity");
    anim->setDuration(duration);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuad);

    connect(anim, &QPropertyAnimation::finished, eff, &QObject::deleteLater);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
void AACScreen::registerInteractive(QWidget* w, bool primary)
{
    if (!w)
        return;

    m_interactive << w;
    if (primary)
        m_primary << w;

    w->installEventFilter(this);
}

QList<QWidget*> AACScreen::interactiveWidgets() const
{
    return m_interactive;
}

QList<QWidget*> AACScreen::primaryWidgets() const
{
    return m_primary;
}

QLayout* AACScreen::rootLayout() const
{
    return layout();
}

bool AACScreen::eventFilter(QObject* obj, QEvent* event)
{
    if (!m_aac)
        return QWidget::eventFilter(obj, event);

    QWidget* w = qobject_cast<QWidget*>(obj);
    if (!w)
        return QWidget::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::Enter:
        m_aac->inputController()->startDwellOn(w);
        break;
    case QEvent::Leave:
        m_aac->inputController()->stopDwellOn(w);
        break;
    default:
        break;
    }

    return QWidget::eventFilter(obj, event);
}

void AACScreen::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);

    if (m_aac) {
        m_aac->inputController()->attachScreen(this);
        m_aac->layoutEngine()->applyLayout(this);
    }
}

void AACScreen::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);

    if (m_aac)
        m_aac->inputController()->detachScreen(this);
}
