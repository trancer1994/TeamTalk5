#include "AACEventMetadataScreen.h"
#include <QVBoxLayout>

static QColor semanticColorForTag(const QString& tag, bool emergency)
{
    if (emergency)
        return QColor("#FF3333"); // bright emergency red

    if (tag == "need_help")
        return QColor("#FF6666"); // soft red

    if (tag == "emotion_happy")
        return QColor("#66CC66"); // green

    if (tag == "emotion_sad")
        return QColor("#6699FF"); // blue

    if (tag == "system")
        return QColor("#AAAAAA"); // grey

    return QColor("#CCCCCC"); // default neutral
}
static QColor semanticBackgroundForTag(const QString& tag, bool emergency)
{
    if (emergency)
        return QColor("#FFCCCC"); // pale emergency red

    if (tag == "need_help")
        return QColor("#FFE5E5"); // pale soft red

    if (tag == "emotion_happy")
        return QColor("#E6FFE6"); // pale green

    if (tag == "emotion_sad")
        return QColor("#E6EEFF"); // pale blue

    if (tag == "system")
        return QColor("#F2F2F2"); // pale grey

    return QColor("#F7F7F7");     // neutral
}
static QString semanticIconForTag(const QString& tag, bool emergency)
{
    if (emergency)
        return "🚨";   // emergency

    if (tag == "need_help")
        return "🆘";   // help

    if (tag == "emotion_happy")
        return "😊";   // happy

    if (tag == "emotion_sad")
        return "😢";   // sad

    if (tag == "system")
        return "⚙️";   // system

    return "🔹";       // neutral/default
}
static QColor semanticBorderForTag(const QString& tag, bool emergency)
{
    if (emergency)
        return QColor("#CC0000"); // strong emergency red

    if (tag == "need_help")
        return QColor("#CC4444"); // strong soft red

    if (tag == "emotion_happy")
        return QColor("#33AA33"); // strong green

    if (tag == "emotion_sad")
        return QColor("#3366CC"); // strong blue

    if (tag == "system")
        return QColor("#888888"); // strong grey

    return QColor("#999999");     // neutral
}
AACEventMetadataScreen::AACEventMetadataScreen(
        const AACConversationRecorderQtAdapter::Event& ev,
        AACAccessibilityManager* aac,
        QWidget* parent)
    : AACScreenBase(aac, parent)
    , m_event(ev)
{
// Determine initial bookmark state
const auto& events = aac->recorder()->events();
for (int i = 0; i < events.size(); ++i) {
    if (&events[i] == &ev) {
        m_isBookmarked = aac->recorder()->isBookmarked(i);
        break;
    }
}
    setScreenTitle(tr("Event Details"));

    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);
root->setContentsMargins(12, 12, 12, 12);

    //
    // Timestamp
    //
    m_labelTimestamp = new AACKeyButton(this);
    m_labelTimestamp->setText(tr("Timestamp: %1")
                              .arg(ev.timestamp.toString("hh:mm:ss")));
    m_labelTimestamp->setAccessibleName(tr("Event timestamp"));
    root->addWidget(m_labelTimestamp);

    //
    // Event type
    //
    QString typeText;
    switch (ev.type) {
    case AACConversationRecorderQtAdapter::EventType::AACMessageSent:
        typeText = tr("AAC message sent");
        break;
    case AACConversationRecorderQtAdapter::EventType::AACMessageSpoken:
        typeText = tr("AAC message spoken");
        break;
    case AACConversationRecorderQtAdapter::EventType::AudioFromUser:
        typeText = tr("Audio from user");
        break;
    case AACConversationRecorderQtAdapter::EventType::AudioToUser:
        typeText = tr("Audio to user");
        break;
    default:
        typeText = tr("Event");
        break;
    }

    m_labelType = new AACKeyButton(this);
    m_labelType->setText(tr("Type: %1").arg(typeText));
    m_labelType->setAccessibleName(tr("Event type"));
    root->addWidget(m_labelType);

    //
    // Text content
    //
    m_labelText = new AACKeyButton(this);
    m_labelText->setText(tr("Content: %1").arg(ev.text.isEmpty()
                                               ? tr("(none)")
                                               : ev.text));
    m_labelText->setAccessibleName(tr("Event content"));
    root->addWidget(m_labelText);

    //
    // Direction
    //
    QString directionText;
    if (ev.type == AACConversationRecorderQtAdapter::EventType::AudioToUser)
        directionText = tr("Audio to user");
    else if (ev.type == AACConversationRecorderQtAdapter::EventType::AudioFromUser)
        directionText = tr("Audio from user");
    else
        directionText = tr("Not applicable");

    m_labelDirection = new AACKeyButton(this);
    m_labelDirection->setText(tr("Direction: %1").arg(directionText));
    m_labelDirection->setAccessibleName(tr("Audio direction"));
    root->addWidget(m_labelDirection);

QString icon = semanticIconForTag(ev.semanticTag, ev.emergency);

m_labelTimestamp->setText(icon + " " +
    tr("Timestamp: %1").arg(ev.timestamp.toString("hh:mm:ss")));

m_labelType->setText(icon + " " +
    tr("Type: %1").arg(typeText));

m_labelText->setText(icon + " " +
    tr("Content: %1").arg(ev.text.isEmpty() ? tr("(none)") : ev.text));

m_labelDirection->setText(icon + " " +
    tr("Direction: %1").arg(directionText));

QColor fg = semanticColorForTag(ev.semanticTag, ev.emergency);
QColor bg = semanticBackgroundForTag(ev.semanticTag, ev.emergency);
QColor bd = semanticBorderForTag(ev.semanticTag, ev.emergency);
auto labelStyle = QString(
    "color: %1; "
    "background-color: %2; "
    "border: 2px solid %3; "
    "border-radius: 6px; "
    "padding: 6px; "
    "margin: 2px; "
    "transition: background-color 150ms ease-in-out; "
    "AACKeyButton:hover { background-color: %4; }")
    .arg(fg.name(), bg.name(), bd.name(), bg.lighter(110).name());

m_labelTimestamp->setStyleSheet(labelStyle);
m_labelType->setStyleSheet(labelStyle);
m_labelText->setStyleSheet(labelStyle);
m_labelDirection->setStyleSheet(labelStyle);

m_waveform = new AACWaveformPreview(this);

// semantic tag colour
m_waveform->setForegroundColor(fg);

// background stays black for contrast
m_waveform->setBackgroundColor(Qt::black);
m_waveform->setStyleSheet(QString("border: 2px solid %1; border-radius: 6px")
                          .arg(bd.name()));

// Example: use event audio samples if available
if (!ev.audioSamples.isEmpty())
    m_waveform->setSamples(ev.audioSamples);
else if (!ev.audioPcm.isEmpty() && ev.audioSampleRate > 0) {
    QVector<float> samples;
    const qint16* pcm = reinterpret_cast<const qint16*>(ev.audioPcm.constData());
    int count = ev.audioPcm.size() / sizeof(qint16);
    samples.reserve(count);
    for (int i = 0; i < count; ++i)
        samples << pcm[i] / 32768.0f;
    m_waveform->setSamples(samples);
}
connect(m_waveform, &AACKeyButton::activated, this, [this]() {
    emit requestReplayEvent(m_event);
});

root->addWidget(m_waveform);

    //
    // Replay button
    //
    m_buttonReplay = new AACKeyButton(this);
    m_buttonReplay->setText(tr("Replay event audio"));
    m_buttonReplay->setAccessibleName(tr("Replay event audio"));
    connect(m_buttonReplay, &AACKeyButton::activated, this, [this]() {
        emit requestReplayEvent(m_event);
    });
    root->addWidget(m_buttonReplay);

    //
    // Export button
    //
    m_buttonExport = new AACKeyButton(this);
    m_buttonExport->setText(tr("Export event audio"));
    m_buttonExport->setAccessibleName(tr("Export event audio"));
    connect(m_buttonExport, &AACKeyButton::activated, this, [this]() {
        emit requestExportEvent(m_event);
    });
    root->addWidget(m_buttonExport);

    //
    // Close button
    //
    m_buttonClose = new AACKeyButton(this);
    m_buttonClose->setText(tr("Close"));
    m_buttonClose->setAccessibleName(tr("Close event details"));
    connect(m_buttonClose, &AACKeyButton::activated, this, [this]() {
        close();
    });
    root->addWidget(m_buttonClose);

QString actionStyle = QString(
    "border: 2px solid #CCCCCC; "
    "border-radius: 6px; "
    "padding: 6px; "
    "margin: 2px; "
    "transition: background-color 150ms ease-in-out; "
    "AACKeyButton:hover { "
        "background-color: #EEEEEE; "
    "}");

auto* jumpToBookmarkButton = new AACKeyButton(this);
jumpToBookmarkButton->setText(tr("Jump to this bookmark"));
jumpToBookmarkButton->setAccessibleName(tr("Jump to this bookmark"));
jumpToBookmarkButton->setStyleSheet(actionStyle);
connect(jumpToBookmarkButton, &AACKeyButton::activated, this, [this]() {
    emit requestJumpToEvent(m_event);
});

// ⭐ Store the pointer here
m_jumpToBookmarkButton = jumpToBookmarkButton;
root->addWidget(jumpToBookmarkButton);

// ⭐ Bookmark toggle (stateful)
m_bookmarkToggleButton = new AACKeyButton(this);

auto updateBookmarkToggleUI = [this]() {
    if (m_isBookmarked) {
        m_bookmarkToggleButton->setText("❌ Remove bookmark");
        m_bookmarkToggleButton->setAccessibleName("Remove bookmark from this event");
        m_bookmarkToggleButton->setStyleSheet("border: 2px solid #CC4444; border-radius: 6px; padding: 6px;");
    } else {
        m_bookmarkToggleButton->setText("⭐ Add bookmark");
        m_bookmarkToggleButton->setAccessibleName("Add bookmark to this event");
        m_bookmarkToggleButton->setStyleSheet("border: 2px solid #33AA33; border-radius: 6px; padding: 6px;");
    }
};

updateBookmarkToggleUI();

connect(m_bookmarkToggleButton, &AACKeyButton::activated, this, [this, updateBookmarkToggleUI]() {
    emit requestToggleBookmark(m_event);
    m_isBookmarked = !m_isBookmarked;
    updateBookmarkToggleUI();
});

root->addWidget(m_bookmarkToggleButton);

m_buttonReplay->setStyleSheet(actionStyle);
m_buttonExport->setStyleSheet(actionStyle);
m_buttonClose->setStyleSheet(actionStyle);

    root->addStretch(1);
}

QList<QWidget*> AACEventMetadataScreen::interactiveWidgets() const
{
    return {
        m_labelTimestamp,
        m_labelType,
        m_labelText,
        m_labelDirection,
m_waveform,
        m_buttonReplay,
        m_buttonExport,
        m_buttonClose,
        m_jumpToBookmarkButton,
        m_bookmarkToggleButton
    };
}

QList<QWidget*> AACEventMetadataScreen::primaryWidgets() const
{
    return interactiveWidgets();
}
