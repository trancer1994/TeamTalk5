#include "AACReplayLastNScreen.h"
#include <QVBoxLayout>

AACReplayLastNScreen::AACReplayLastNScreen(
        AACConversationRecorderQtAdapter* recorder,
        AACAccessibilityManager* aac,
        QWidget* parent)
    : AACScreenBase(aac, parent)
    , m_recorder(recorder)
{
    setScreenTitle(tr("Replay Options"));

    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);

    const auto events = recorder->events();

    //
    // Helper lambda to slice last N events
    //
    auto sliceLast = [&](int n) {
        QVector<AACConversationRecorderQtAdapterQtAdapter::Event> out;
        int start = qMax(0, events.size() - n);
        for (int i = start; i < events.size(); ++i)
            out << events[i];
        return out;
    };

    //
    // Replay last 3
    //
    m_buttonLast3 = new AACKeyButton(this);
    m_buttonLast3->setText(tr("Replay last 3 events"));
    connect(m_buttonLast3, &AACKeyButton::activated, this, [this, sliceLast]() {
        emit requestReplayEvents(sliceLast(3));
    });
    root->addWidget(m_buttonLast3);

    //
    // Replay last 5
    //
    m_buttonLast5 = new AACKeyButton(this);
    m_buttonLast5->setText(tr("Replay last 5 events"));
    connect(m_buttonLast5, &AACKeyButton::activated, this, [this, sliceLast]() {
        emit requestReplayEvents(sliceLast(5));
    });
    root->addWidget(m_buttonLast5);

    //
    // Replay last 10
    //
    m_buttonLast10 = new AACKeyButton(this);
    m_buttonLast10->setText(tr("Replay last 10 events"));
    connect(m_buttonLast10, &AACKeyButton::activated, this, [this, sliceLast]() {
        emit requestReplayEvents(sliceLast(10));
    });
    root->addWidget(m_buttonLast10);

    //
    // Replay last spoken message
    //
    m_buttonLastSpoken = new AACKeyButton(this);
    m_buttonLastSpoken->setText(tr("Replay last spoken message"));
    connect(m_buttonLastSpoken, &AACKeyButton::activated, this, [this]() {
        m_recorder->playLastSpokenMessage();
    });
    root->addWidget(m_buttonLastSpoken);

    //
    // Replay last AAC message
    //
    m_buttonLastAAC = new AACKeyButton(this);
    m_buttonLastAAC->setText(tr("Replay last AAC message"));
    connect(m_buttonLastAAC, &AACKeyButton::activated, this, [this]() {
        m_recorder->playLastAACMessage();
    });
    root->addWidget(m_buttonLastAAC);

    //
    // Replay last audio frame
    //
    m_buttonLastAudioFrame = new AACKeyButton(this);
    m_buttonLastAudioFrame->setText(tr("Replay last audio frame"));
    connect(m_buttonLastAudioFrame, &AACKeyButton::activated, this, [this]() {
        m_recorder->playLastAudioFrame();
    });
    root->addWidget(m_buttonLastAudioFrame);

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

QList<QWidget*> AACReplayLastNScreen::interactiveWidgets() const
{
    return {
        m_buttonLast3,
        m_buttonLast5,
        m_buttonLast10,
        m_buttonLastSpoken,
        m_buttonLastAAC,
        m_buttonLastAudioFrame,
        m_buttonClose
    };
}

QList<QWidget*> AACReplayLastNScreen::primaryWidgets() const
{
    return interactiveWidgets();
}
