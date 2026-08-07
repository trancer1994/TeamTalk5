#include "AACConversationSummaryScreen.h"
#include <QVBoxLayout>

AACConversationSummaryScreen::AACConversationSummaryScreen(
        const QVector<AACConversationRecorder::Event>& events,
        AACAccessibilityManager* aac,
        QWidget* parent)
    : AACScreenBase(aac, parent)
    , m_events(events)
{
    setScreenTitle(tr("Conversation Summary"));

    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);

    int total = events.size();
    int aacSent = 0;
    int aacSpoken = 0;
    int audioToUser = 0;
    int audioFromUser = 0;

    QTime firstTime, lastTime;
    bool haveTime = false;

    for (const auto& ev : events) {
        switch (ev.type) {
        case AACConversationRecorder::EventType::AACMessageSent:     aacSent++; break;
        case AACConversationRecorder::EventType::AACMessageSpoken:   aacSpoken++; break;
        case AACConversationRecorder::EventType::AudioToUser:        audioToUser++; break;
        case AACConversationRecorder::EventType::AudioFromUser:      audioFromUser++; break;
        default: break;
        }

        if (!haveTime) {
            firstTime = ev.timestamp;
            lastTime = ev.timestamp;
            haveTime = true;
        } else {
            if (ev.timestamp < firstTime) firstTime = ev.timestamp;
            if (ev.timestamp > lastTime) lastTime = ev.timestamp;
        }
    }

    int durationSec = haveTime ? firstTime.secsTo(lastTime) : 0;
    QString durationText = haveTime
        ? tr("%1 seconds").arg(durationSec)
        : tr("Unknown");

    //
    // Summary labels
    //
    m_labelTotal = new AACKeyButton(this);
    m_labelTotal->setText(tr("Total events: %1").arg(total));
    root->addWidget(m_labelTotal);

    m_labelAACSent = new AACKeyButton(this);
    m_labelAACSent->setText(tr("AAC messages sent: %1").arg(aacSent));
    root->addWidget(m_labelAACSent);

    m_labelAACSpoken = new AACKeyButton(this);
    m_labelAACSpoken->setText(tr("AAC messages spoken: %1").arg(aacSpoken));
    root->addWidget(m_labelAACSpoken);

    m_labelAudioToUser = new AACKeyButton(this);
    m_labelAudioToUser->setText(tr("Audio to user: %1").arg(audioToUser));
    root->addWidget(m_labelAudioToUser);

    m_labelAudioFromUser = new AACKeyButton(this);
    m_labelAudioFromUser->setText(tr("Audio from user: %1").arg(audioFromUser));
    root->addWidget(m_labelAudioFromUser);

    m_labelDuration = new AACKeyButton(this);
    m_labelDuration->setText(tr("Total duration: %1").arg(durationText));
    root->addWidget(m_labelDuration);

QColor fg = semanticColorForTag("", false);          // neutral foreground
QColor bg = semanticBackgroundForTag("", false);     // neutral background
QColor bd = semanticBorderForTag("", false);         // neutral border

auto style = QString(
    "color: %1; "
    "background-color: %2; "
    "border: 2px solid %3; "
    "border-radius: 6px; "
    "padding: 6px; "
    "margin: 2px; "
    "transition: background-color 150ms ease-in-out; "
    "AACKeyButton:hover { "
        "background-color: %4; "
    "}")
    .arg(fg.name(), bg.name(), bd.name(), bg.lighter(110).name());

m_labelTotal->setStyleSheet(style);
m_labelAACSent->setStyleSheet(style);
m_labelAACSpoken->setStyleSheet(style);
m_labelAudioToUser->setStyleSheet(style);
m_labelAudioFromUser->setStyleSheet(style);
m_labelDuration->setStyleSheet(style);

    //
    // Action buttons
    //
    m_buttonReplay = new AACKeyButton(this);
    m_buttonReplay->setText(tr("Replay entire conversation"));
    connect(m_buttonReplay, &AACKeyButton::activated, this, [this]() {
        emit requestReplayConversation();
    });
    root->addWidget(m_buttonReplay);

    m_buttonExport = new AACKeyButton(this);
    m_buttonExport->setText(tr("Export conversation audio"));
    connect(m_buttonExport, &AACKeyButton::activated, this, [this]() {
        emit requestExportConversation();
    });
    root->addWidget(m_buttonExport);

    m_buttonClose = new AACKeyButton(this);
    m_buttonClose->setText(tr("Close"));
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
m_buttonReplay->setStyleSheet(actionStyle);
m_buttonExport->setStyleSheet(actionStyle);
m_buttonClose->setStyleSheet(actionStyle);

    root->addStretch(1);
}

QList<QWidget*> AACConversationSummaryScreen::interactiveWidgets() const
{
    return {
        m_labelTotal,
        m_labelAACSent,
        m_labelAACSpoken,
        m_labelAudioToUser,
        m_labelAudioFromUser,
        m_labelDuration,
        m_buttonReplay,
        m_buttonExport,
        m_buttonClose
    };
}

QList<QWidget*> AACConversationSummaryScreen::primaryWidgets() const
{
    return interactiveWidgets();
}
