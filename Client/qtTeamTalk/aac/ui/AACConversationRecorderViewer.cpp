#include "AACConversationRecorderViewer.h"
#include <QFileDialog>
#include <QFile>

AACConversationRecorderViewer::AACConversationRecorderViewer(
        AACConversationRecorderQtAdapter* recorder,
        AACAccessibilityManager* aac,
        QWidget* parent)
    : AACScreenBase(aac, parent)
    , m_recorder(recorder)
{
    setScreenTitle(tr("Conversation Recorder"));

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);

    m_scrollWidget = new QWidget(this);
    m_listLayout   = new QVBoxLayout(m_scrollWidget);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(6);

    m_scrollArea->setWidget(m_scrollWidget);

    auto* root = new QVBoxLayout(this);
auto* controls = new QVBoxLayout();

// --- Search bar ---
auto* searchField = new QLineEdit(this);
auto* searchButton = new AACKeyButton(this);
searchButton->setText(tr("Search"));

connect(searchButton, &AACKeyButton::activated, this, [this, searchField]() {
    QList<int> hits = m_recorder->searchText(searchField->text());
    rebuildListFromIndices(hits);
});

controls->addWidget(searchField);
controls->addWidget(searchButton);

// --- Filter buttons ---
auto* filterHelpButton = new AACKeyButton(this);
filterHelpButton->setText(tr("Filter: need_help"));
connect(filterHelpButton, &AACKeyButton::activated, this, [this]() {
    rebuildListFromIndices(m_recorder->filterByTag("need_help"));
});
controls->addWidget(filterHelpButton);

// --- Jump navigation ---
auto* jumpSadButton = new AACKeyButton(this);
jumpSadButton->setText(tr("Jump to next sad event"));
connect(jumpSadButton, &AACKeyButton::activated, this, [this]() {
    int idx = m_recorder->nextEventWithTag("emotion_sad", /*currentIndex*/ 0);
    scrollToIndex(idx);
});
controls->addWidget(jumpSadButton);

// --- Bookmarks ---
auto* bookmarkViewButton = new AACKeyButton(this);
bookmarkViewButton->setText(tr("Show bookmarks"));
connect(bookmarkViewButton, &AACKeyButton::activated, this, [this]() {
    rebuildListFromIndices(m_recorder->bookmarkedEvents());
});
controls->addWidget(bookmarkViewButton);

// --- Jump toolbar ---

auto* jumpFirstButton = new AACKeyButton(this);
jumpFirstButton->setText(tr("Jump to first event"));
connect(jumpFirstButton, &AACKeyButton::activated, this, [this]() {
    scrollToIndex(0);
});
controls->addWidget(jumpFirstButton);

auto* jumpLastButton = new AACKeyButton(this);
jumpLastButton->setText(tr("Jump to last event"));
connect(jumpLastButton, &AACKeyButton::activated, this, [this]() {
    scrollToIndex(m_recorder->events().size() - 1);
});
controls->addWidget(jumpLastButton);

auto* jumpFirstAACButton = new AACKeyButton(this);
jumpFirstAACButton->setText(tr("Jump to first AAC message"));
connect(jumpFirstAACButton, &AACKeyButton::activated, this, [this]() {
    int idx = m_recorder->firstEventOfType(EventType::AACMessageSent);
    if (idx < 0)
        idx = m_recorder->firstEventOfType(EventType::AACMessageSpoken);
    scrollToIndex(idx);
});
controls->addWidget(jumpFirstAACButton);

auto* jumpLastAACButton = new AACKeyButton(this);
jumpLastAACButton->setText(tr("Jump to last AAC message"));
connect(jumpLastAACButton, &AACKeyButton::activated, this, [this]() {
    int idx = m_recorder->lastEventOfType(EventType::AACMessageSent);
    if (idx < 0)
        idx = m_recorder->lastEventOfType(EventType::AACMessageSpoken);
    scrollToIndex(idx);
});
controls->addWidget(jumpLastAACButton);

auto* jumpFirstAudioButton = new AACKeyButton(this);
jumpFirstAudioButton->setText(tr("Jump to first audio event"));
connect(jumpFirstAudioButton, &AACKeyButton::activated, this, [this]() {
    int idx = m_recorder->firstEventOfType(EventType::AudioFromUser);
    if (idx < 0)
        idx = m_recorder->firstEventOfType(EventType::AudioToUser);
    scrollToIndex(idx);
});
controls->addWidget(jumpFirstAudioButton);

auto* jumpLastAudioButton = new AACKeyButton(this);
jumpLastAudioButton->setText(tr("Jump to last audio event"));
connect(jumpLastAudioButton, &AACKeyButton::activated, this, [this]() {
    int idx = m_recorder->lastEventOfType(EventType::AudioFromUser);
    if (idx < 0)
        idx = m_recorder->lastEventOfType(EventType::AudioToUser);
    scrollToIndex(idx);
});
controls->addWidget(jumpLastAudioButton);

auto* semanticNavButton = new AACKeyButton(this);
semanticNavButton->setText(tr("Semantic timeline"));
connect(semanticNavButton, &AACKeyButton::activated, this, [this]() {
    auto* screen = new AACSemanticTimelineScreen(m_recorder, aacManager(), this);
    connect(screen, &AACSemanticTimelineScreen::requestJumpToIndex,
            this, &AACConversationRecorderViewer::scrollToIndex);
    screen->show();
});
controls->addWidget(semanticNavButton);
m_semanticNavButton = semanticNavButton;

auto* bookmarkManagerButton = new AACKeyButton(this);
bookmarkManagerButton->setText(tr("Manage bookmarks"));
connect(bookmarkManagerButton, &AACKeyButton::activated, this, [this]() {
    auto* screen = new AACBookmarkManagerScreen(m_recorder, aacManager(), this);
    connect(screen, &AACBookmarkManagerScreen::requestJumpToIndex,
            this, &AACConversationRecorderViewer::scrollToIndex);
    screen->show();
});
controls->addWidget(bookmarkManagerButton);
m_bookmarkManagerButton = bookmarkManagerButton;

auto* jumpHistoryButton = new AACKeyButton(this);
jumpHistoryButton->setText(tr("Jump history"));
connect(jumpHistoryButton, &AACKeyButton::activated, this, [this]() {
    auto* screen = new AACJumpHistoryScreen(
        m_jumpHistory,
        m_recorder->events(),
        aacManager(),
        this
    );
    connect(screen, &AACJumpHistoryScreen::requestJumpToIndex,
            this, &AACConversationRecorderViewer::scrollToIndex);
    screen->show();
});
controls->addWidget(jumpHistoryButton);
m_jumpHistoryButton = jumpHistoryButton;

// Add controls to root
root->addLayout(controls);
    root->addWidget(m_scrollArea);

    if (m_recorder)
        onConversationUpdated(m_recorder->events());

    if (m_recorder)
        connect(m_recorder, &AACConversationRecorderQtAdapter::conversationUpdated,
                this, &AACConversationRecorderViewer::onConversationUpdated);

connect(this, &AACConversationRecorderViewer::requestShowDetails,
        this, [this](const AACConversationRecorderQtAdapter::Event& ev) {
            auto* screen = new AACEventMetadataScreen(ev, aacManager(), this);

            // ⭐ Connect bookmark jump
            connect(screen, &AACEventMetadataScreen::requestJumpToEvent,
                    this, &AACConversationRecorderViewer::scrollToEvent);

connect(screen, &AACEventMetadataScreen::requestToggleBookmark,
        this, [this](const AACConversationRecorderQtAdapter::Event& ev2) {
            toggleBookmarkForEvent(ev2);
        });
            screen->show();
        });
connect(this, &AACConversationRecorderViewer::requestExportConversation,
        this, [this]() {
            auto* screen = new AACConversationSummaryScreen(
                m_recorder->events(),
                aacManager(),
                this
            );
            screen->show();
        });
}

void AACConversationRecorderViewer::onConversationUpdated(
        const QVector<AACConversationRecorderQtAdapter::Event>& events)
{
m_seekButtons.clear();
    for (auto* item : m_items)
        item->deleteLater();
    m_items.clear();

    //
    // ⭐ 1. Add event rows (AACConversationRecorderItem)
    //
    for (int i = 0; i < events.size(); ++i) {
        const auto& ev = events[i];

        auto* item = new AACConversationRecorderItem(ev, this);
        m_items << item;
        m_listLayout->addWidget(item);

        // Play entire conversation
        connect(item, &AACConversationRecorderItem::requestPlay,
                this, [this]() {
                    if (m_recorder)
                        m_recorder->playConversationAudio();
                    emit requestPlayConversation();
                });

        // Show event metadata
connect(item, &AACConversationRecorderItem::requestDetails,
        this, [this](const AACConversationRecorderQtAdapter::Event& ev) {
            auto* screen = new AACEventMetadataScreen(ev, aacManager(), this);

            connect(screen, &AACEventMetadataScreen::requestJumpToEvent,
                    this, &AACConversationRecorderViewer::scrollToEvent);

            connect(screen, &AACEventMetadataScreen::requestToggleBookmark,
                    this, [this](const AACConversationRecorderQtAdapter::Event& ev2) {
                        toggleBookmarkForEvent(ev2);
                    });
            screen->show();
        });

        // Export entire conversation WAV
        connect(item, &AACConversationRecorderItem::requestExport,
                this, [this]() {
                    if (!m_recorder)
                        return;

                    QByteArray wav = m_recorder->exportConversationAudioWav();
                    if (wav.isEmpty())
                        return;

                    QString path = QFileDialog::getSaveFileName(
                        this,
                        tr("Export Conversation Audio"),
                        "conversation.wav",
                        tr("WAV files (*.wav)")
                    );

                    if (!path.isEmpty()) {
                        QFile f(path);
                        if (f.open(QIODevice::WriteOnly))
                            f.write(wav);
                    }

                    emit requestExportConversation();
                });

        //
        // ⭐ 2. Event‑based seeking button (AACEventSeekButton)
        //
        auto* seekButton = new AACEventSeekButton(ev, i, this);
m_seekButtons << seekButton;

        connect(seekButton, &AACEventSeekButton::seekToEvent,
                m_recorder, &AACConversationRecorderQtAdapter::playEventAudio);

        connect(seekButton, &AACEventSeekButton::showEventDetails,
                this, &AACConversationRecorderViewer::requestShowDetails);

        m_listLayout->addWidget(seekButton);
    }

    //
    // ⭐ 3. Jump‑to‑last‑AAC‑message button
    //
m_jumpLastMessage = new AACJumpLastMessageButton(this);
connect(m_jumpLastMessage, &AACJumpLastMessageButton::jumpToLastAACMessage,
        m_recorder, &AACConversationRecorderQtAdapter::playLastAACMessage);
m_listLayout->addWidget(m_jumpLastMessage);

    //
    // ⭐ 4. Jump‑to‑last‑audio‑frame button
    //
m_jumpLastAudioFrame = new AACJumpLastAudioFrameButton(this);
connect(m_jumpLastAudioFrame, &AACJumpLastAudioFrameButton::jumpToLastAudioFrame,
        m_recorder, &AACConversationRecorderQtAdapter::playLastAudioFrame);
m_listLayout->addWidget(m_jumpLastAudioFrame);

//
// ⭐ 5. Conversation summary button
//
m_summaryButton = new AACKeyButton(this);
m_summaryButton->setText(tr("Show conversation summary"));

connect(m_summaryButton, &AACKeyButton::activated, this, [this]() {
    auto* summary = new AACConversationSummaryScreen(
        m_recorder->events(),
        aacManager(),
        this
    );
    summary->show();
});

m_listLayout->addWidget(m_summaryButton);

m_replayOptionsButton = new AACKeyButton(this);
m_replayOptionsButton->setText(tr("Replay options"));

connect(m_replayOptionsButton, &AACKeyButton::activated, this, [this]() {
    auto* screen = new AACReplayLastNScreen(
        m_recorder,
        aacManager(),
        this
    );
    screen->show();
});

m_listLayout->addWidget(m_replayOptionsButton);

m_exportOptionsButton = new AACKeyButton(this);
m_exportOptionsButton->setText(tr("Export options"));

connect(m_exportOptionsButton, &AACKeyButton::activated, this, [this]() {
    auto* screen = new AACExportOptionsScreen(
        m_recorder,
        aacManager(),
        this
    );
    screen->show();
});

m_listLayout->addWidget(m_exportOptionsButton);

    //
    // ⭐ Keep your stretch
    //
    m_listLayout->addStretch(1);
}
QList<QWidget*> AACConversationRecorderViewer::interactiveWidgets() const
{
    QList<QWidget*> out;

    // Event rows
    for (auto* item : m_items)
        out << item;

    // Event seek buttons
    for (auto* btn : m_seekButtons)
        out << btn;

    // Jump buttons
    if (m_jumpLastMessage)
        out << m_jumpLastMessage;

    if (m_jumpLastAudioFrame)
        out << m_jumpLastAudioFrame;

if (m_summaryButton)
    out << m_summaryButton;

if (m_replayOptionsButton)
    out << m_replayOptionsButton;

if (m_exportOptionsButton)
    out << m_exportOptionsButton;

if (m_semanticNavButton)
    out << m_semanticNavButton;

if (m_bookmarkManagerButton)
    out << m_bookmarkManagerButton;

if (m_jumpHistoryButton)
    out << m_jumpHistoryButton;

    return out;
}

QList<QWidget*> AACConversationRecorderViewer::primaryWidgets() const
{
    return interactiveWidgets();
}

QString AACConversationRecorderViewer::contextualHelp() const
{
    return tr(
        "Recorder:\n"
        "Use scanning or arrow keys to navigate events.\n"
        "Press Enter to play the conversation or view details.\n"
        "Press Escape to return to the main screen."
    );
}

void AACConversationRecorderViewer::rebuildListFromIndices(const QList<int>& indices)
{
    m_seekButtons.clear();

    for (auto* item : m_items)
        item->deleteLater();
    m_items.clear();

    for (int idx : indices) {
        const auto& ev = m_recorder->events()[idx];

        auto* item = new AACConversationRecorderItem(ev, this);
        m_items << item;
        m_listLayout->addWidget(item);

        connect(item, &AACConversationRecorderItem::requestPlay,
                this, [this]() {
                    if (m_recorder)
                        m_recorder->playConversationAudio();
                    emit requestPlayConversation();
                });

        connect(item, &AACConversationRecorderItem::requestDetails,
                this, &AACConversationRecorderViewer::requestShowDetails);

        connect(item, &AACConversationRecorderItem::requestExport,
                this, [this]() {
                    QByteArray wav = m_recorder->exportConversationAudioWav();
                    if (wav.isEmpty())
                        return;

                    QString path = QFileDialog::getSaveFileName(
                        this,
                        tr("Export Conversation Audio"),
                        "conversation.wav",
                        tr("WAV files (*.wav)")
                    );

                    if (!path.isEmpty()) {
                        QFile f(path);
                        if (f.open(QIODevice::WriteOnly))
                            f.write(wav);
                    }

                    emit requestExportConversation();
                });

        auto* seekButton = new AACEventSeekButton(ev, idx, this);
        m_seekButtons << seekButton;

        connect(seekButton, &AACEventSeekButton::seekToEvent,
                m_recorder, &AACConversationRecorderQtAdapter::playEventAudio);

        connect(seekButton, &AACEventSeekButton::showEventDetails,
                this, &AACConversationRecorderViewer::requestShowDetails);

        m_listLayout->addWidget(seekButton);
    }

    m_listLayout->addStretch(1);
}
void AACConversationRecorderViewer::refreshScanning()
{
    rebuildInteractiveGroup(interactiveWidgets());
    rebuildPrimaryGroup(primaryWidgets());
}
void AACConversationRecorderViewer::scrollToIndex(int index)
{
    if (index < 0 || index >= m_items.size())
        return;

    m_jumpHistory << index; // track jumps

    QWidget* target = m_items[index];
    if (!target)
        return;

    // Ensure layout is updated before computing geometry
    m_scrollWidget->updateGeometry();
    m_scrollArea->updateGeometry();
    m_scrollArea->viewport()->updateGeometry();

    // Scroll so the target widget is visible
    m_scrollArea->ensureWidgetVisible(target, 20, 20);
}
void AACConversationRecorderViewer::scrollToEvent(const AACConversationRecorderQtAdapter::Event& ev)
{
    // Find the event index
    const auto& events = m_recorder->events();
    for (int i = 0; i < events.size(); ++i) {
        if (&events[i] == &ev) {
            scrollToIndex(i);
            return;
        }
    }
}
void AACConversationRecorderViewer::toggleBookmarkForEvent(
        const AACConversationRecorderQtAdapter::Event& ev)
{
    const auto& events = m_recorder->events();
    for (int i = 0; i < events.size(); ++i) {
        if (&events[i] == &ev) {
            if (m_recorder->isBookmarked(i))
                m_recorder->removeBookmark(i);
            else
                m_recorder->addBookmark(i);
            refreshScanning();
            return;
        }
    }
}
void AACConversationRecorderViewer::toggleBookmarkForEvent(
        const AACConversationRecorderQtAdapter::Event& ev)
{
    const auto& events = m_recorder->events();
    for (int i = 0; i < events.size(); ++i) {
        if (&events[i] == &ev) {
            if (m_recorder->isBookmarked(i))
                m_recorder->removeBookmark(i);
            else
                m_recorder->addBookmark(i);

            refreshScanning();
            return;
        }
    }
}
