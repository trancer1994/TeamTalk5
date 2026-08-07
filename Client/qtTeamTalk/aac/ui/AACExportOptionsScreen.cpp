#include "AACExportOptionsScreen.h"
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

AACExportOptionsScreen::AACExportOptionsScreen(
AACConversationRecorderQtAdapter* recorder,
        AACAccessibilityManager* aac,
        QWidget* parent)
    : AACScreenBase(aac, parent)
    , m_recorder(recorder)
{
    setScreenTitle(tr("Export Options"));

    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);

    const auto events = recorder->events();

    //
    // Build transcript text
    //
    QString transcript;
    for (const auto& ev : events) {
        transcript += QString("[%1] %2\n")
            .arg(ev.timestamp.toString("hh:mm:ss"))
            .arg(ev.text.isEmpty() ? tr("(no text)") : ev.text);
    }

    //
    // Build JSON event list
    //
    QJsonArray arr;
    for (const auto& ev : events) {
        QJsonObject obj;
        obj["timestamp"] = ev.timestamp.toString(Qt::ISODate);
        obj["type"] = int(ev.type);
        obj["text"] = ev.text;
        arr.append(obj);
    }
    QByteArray json = QJsonDocument(arr).toJson();

    //
    // Build summary text
    //
    int total = events.size();
    int aacSent = 0;
    int aacSpoken = 0;
    int audioToUser = 0;
    int audioFromUser = 0;

    for (const auto& ev : events) {
        switch (ev.type) {
        case AACConversationRecorderQtAdapter::EventType::AACMessageSent:     aacSent++; break;
        case AACConversationRecorderQtAdapter::EventType::AACMessageSpoken:   aacSpoken++; break;
        case AACConversationRecorderQtAdapter::EventType::AudioToUser:        audioToUser++; break;
        case AACConversationRecorderQtAdapter::EventType::AudioFromUser:      audioFromUser++; break;
        default: break;
        }
    }

    QString summary = tr(
        "Total events: %1\n"
        "AAC messages sent: %2\n"
        "AAC messages spoken: %3\n"
        "Audio to user: %4\n"
        "Audio from user: %5\n"
    ).arg(total).arg(aacSent).arg(aacSpoken).arg(audioToUser).arg(audioFromUser);

    //
    // Export transcript
    //
    m_buttonTranscript = new AACKeyButton(this);
    m_buttonTranscript->setText(tr("Export transcript"));
    connect(m_buttonTranscript, &AACKeyButton::activated, this, [this, transcript]() {
        emit requestExportTranscript(transcript);
    });
    root->addWidget(m_buttonTranscript);

    //
    // Export event list (JSON)
    //
    m_buttonEventList = new AACKeyButton(this);
    m_buttonEventList->setText(tr("Export event list (JSON)"));
    connect(m_buttonEventList, &AACKeyButton::activated, this, [this, json]() {
        emit requestExportEventList(json);
    });
    root->addWidget(m_buttonEventList);

    //
    // Export summary
    //
    m_buttonSummary = new AACKeyButton(this);
    m_buttonSummary->setText(tr("Export summary"));
    connect(m_buttonSummary, &AACKeyButton::activated, this, [this, summary]() {
        emit requestExportSummary(summary);
    });
    root->addWidget(m_buttonSummary);

    //
    // Close
    //
    m_buttonClose = new AACKeyButton(this);
    m_buttonClose->setText(tr("Close"));
    connect(m_buttonClose, &AACKeyButton::activated, this, [this]() {
        close();
    });
    root->addWidget(m_buttonClose);

    root->addStretch(1);
}

QList<QWidget*> AACExportOptionsScreen::interactiveWidgets() const
{
    return {
        m_buttonTranscript,
        m_buttonEventList,
        m_buttonSummary,
        m_buttonClose
    };
}

QList<QWidget*> AACExportOptionsScreen::primaryWidgets() const
{
    return interactiveWidgets();
}
