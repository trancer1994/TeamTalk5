#include "AACMessageHistoryViewer.h"
#include "AACMessageHistoryItem.h"

AACMessageHistoryViewer::AACMessageHistoryViewer(AACMessageHistory* history,
                                                 QWidget* parent)
    : AACScreenBase(history ? history->manager() : nullptr, parent)
    , m_history(history)
{
    setScreenTitle(tr("Message History"));

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    // Scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);

    m_scrollWidget = new QWidget(this);
    m_listLayout = new QVBoxLayout(m_scrollWidget);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(6);

    m_scrollArea->setWidget(m_scrollWidget);
    m_rootLayout->addWidget(m_scrollArea);

    // Initial population
    if (m_history)
        onHistoryChanged(m_history->history());

    // Wiring
    if (m_history)
        connect(m_history, &AACMessageHistory::historyChanged,
                this, &AACMessageHistoryViewer::onHistoryChanged);
}

void AACMessageHistoryViewer::onHistoryChanged(const QList<AACMessage>& list)
{
    // Clear old items
    for (auto* item : m_items)
        item->deleteLater();
    m_items.clear();

    // Create new items
    for (const AACMessage& msg : list) {
        auto* item = new AACMessageHistoryItem(msg, this);
        m_items << item;
        m_listLayout->addWidget(item);
    }

    m_listLayout->addStretch(1);
}

QList<QWidget*> AACMessageHistoryViewer::interactiveWidgets() const
{
    QList<QWidget*> out;

    for (auto* item : m_items) {
        out << item->textLabel();
        out << item->replayButton();
        out << item->speakButton();
        out << item->copyButton();
        out << item->deleteButton();
    }

    return out;
}

QList<QWidget*> AACMessageHistoryViewer::primaryWidgets() const
{
    QList<QWidget*> out;

    for (auto* item : m_items) {
        out << item->textLabel();
        out << item->speakButton();
        out << item->replayButton();
    }

    return out;
}

QString AACMessageHistoryViewer::contextualHelp() const
{
    return tr(
        "History:\n"
        "Use scanning or arrow keys to navigate messages.\n"
        "Press Enter to activate an action.\n"
        "Press Escape to return to the Compose tab."
    );
}
void AACMessageHistoryViewer::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
    case Qt::Key_Up:
        jumpToPreviousEvent();
        announceCurrentEvent();
        return;

    case Qt::Key_Down:
        jumpToNextEvent();
        announceCurrentEvent();
        return;

    case Qt::Key_Home:
        jumpToFirstEvent();
        announceCurrentEvent();
        return;

    case Qt::Key_End:
        jumpToLastEvent();
        announceCurrentEvent();
        return;

    case Qt::Key_Escape:
        // Close AAC-modal screen
        if (parentWidget())
            parentWidget()->setVisible(false);
        return;
    }

    QWidget::keyPressEvent(e);
}
void AACMessageHistoryViewer::announceCurrentEvent()
{
    if (!m_aac || !m_aac->speechEngine() || m_aac->modes().fatigueMode)
        return;

    const auto ev = currentEvent(); // your existing API

    m_aac->speechEngine()->speakNotification(
        tr("%1 at %2: %3")
            .arg(eventTypeToString(ev.type))
            .arg(ev.timestamp.toString(Qt::DefaultLocaleShortDate))
            .arg(ev.summary)
    );
}
