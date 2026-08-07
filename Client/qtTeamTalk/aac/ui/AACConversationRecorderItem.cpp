#include "AACConversationRecorderItem.h"

AACConversationRecorderItem::AACConversationRecorderItem(
        const AACConversationRecorderQtAdapter::Event& ev,
        QWidget* parent)
    : AACKeyButton(parent)
    , m_event(ev)
{
// Timestamp (AAC‑friendly)
m_timestamp = ev.timestamp.toString("hh:mm:ss");

// Base label
QString base = ev.text;
if (base.isEmpty()) {
    switch (ev.type) {
    case AACConversationRecorderQtAdapter::EventType::AACMessageSent:     base = tr("AAC message sent"); break;
    case AACConversationRecorderQtAdapter::EventType::AACMessageSpoken:   base = tr("AAC message spoken"); break;
    case AACConversationRecorderQtAdapter::EventType::AudioFromUser:      base = tr("Audio from user"); break;
    case AACConversationRecorderQtAdapter::EventType::AudioToUser:        base = tr("Audio to user"); break;
    default: base = tr("Event");
    }
}

// Final AAC‑idiomatic label with timestamp
QString label = QString("%1  [%2]").arg(base, m_timestamp);
setText(label);
    setAccessibleName(tr("Conversation event"));
QColor fg = semanticColorForTag(ev.semanticTag, ev.emergency);
QColor bg = semanticBackgroundForTag(ev.semanticTag, ev.emergency);

setStyleSheet(QString("color: %1; background-color: %2")
              .arg(fg.name(), bg.name()));

    //
    // AACKeyButton activation semantics:
    // - activated        = primary action (details)
    // - longActivated    = replay audio
    // - doubleActivated  = export conversation
    //

    connect(this, &AACKeyButton::activated, this, [this]() {
        emit requestDetails(m_event);
    });

    connect(this, &AACKeyButton::longActivated, this, [this]() {
        emit requestPlay();
    });

    connect(this, &AACKeyButton::doubleActivated, this, [this]() {
        emit requestExport();
    });
}
