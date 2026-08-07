#include "AACEventSeekButton.h"

AACEventSeekButton::AACEventSeekButton(
        const AACConversationRecorderQtAdapter::Event& ev,
        int index,
        QWidget* parent)
    : AACKeyButton(parent)
    , m_event(ev)
    , m_index(index)
{
    QString icon = semanticIconForTag(ev.semanticTag, ev.emergency);
    QString base = ev.text.isEmpty() ? tr("Event %1").arg(index) : ev.text;
    setText(icon + " " + base);

    setAccessibleName(tr("Seek to event %1").arg(index));

    QColor fg = semanticColorForTag(ev.semanticTag, ev.emergency);
    QColor bg = semanticBackgroundForTag(ev.semanticTag, ev.emergency);

    setStyleSheet(QString("color: %1; background-color: %2")
                  .arg(fg.name(), bg.name()));

    connect(this, &AACKeyButton::activated, this, [this]() {
        emit seekToEvent(m_index);
    });

    connect(this, &AACKeyButton::doubleActivated, this, [this]() {
        emit showEventDetails(m_event);
    });
}