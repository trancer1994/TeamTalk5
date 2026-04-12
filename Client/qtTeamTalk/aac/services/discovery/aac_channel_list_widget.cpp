#include "aac_channel_list_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

AACChannelListWidget::AACChannelListWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    m_debounce.start();
}

void AACChannelListWidget::setupUi()
{
    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_list, &QListWidget::currentItemChanged,
            this, &AACChannelListWidget::onHighlightChanged);

    connect(m_list, &QListWidget::itemActivated,
            this, &AACChannelListWidget::onItemActivated);

    m_joinButton = new QPushButton(tr("Join"), this);
    m_backButton = new QPushButton(tr("Back"), this);
    m_statusLabel = new QLabel(tr("Select a channel"), this);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(m_joinButton);
    buttons->addStretch();
    buttons->addWidget(m_backButton);

    auto* layout = new QVBoxLayout;
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_list);
    layout->addLayout(buttons);

    setLayout(layout);

    connect(m_joinButton, &QPushButton::clicked,
            this, &AACChannelListWidget::onJoinClicked);
    connect(m_backButton, &QPushButton::clicked,
            this, &AACChannelListWidget::backRequested);
}

void AACChannelListWidget::setChannels(const QStringList& channels)
{
    m_transitioning = false;
    m_list->clear();
    m_list->addItems(channels);

    if (!channels.isEmpty())
        m_list->setCurrentRow(0);
}

QString AACChannelListWidget::selectedChannel() const
{
    auto* item = m_list->currentItem();
    return item ? item->text() : QString();
}

bool AACChannelListWidget::acceptInput()
{
    if (m_transitioning)
        return false;

    if (m_debounce.elapsed() < 120)
        return false;

    m_debounce.restart();
    return true;
}

void AACChannelListWidget::onHighlightChanged()
{
    ensureHighlightVisible();
}

void AACChannelListWidget::ensureHighlightVisible()
{
    auto* item = m_list->currentItem();
    if (item)
        m_list->scrollToItem(item, QAbstractItemView::EnsureVisible);
}

void AACChannelListWidget::onItemActivated(QListWidgetItem* item)
{
    if (!item || !acceptInput())
        return;

    m_transitioning = true;
    emit joinRequested(item->text());
}

void AACChannelListWidget::onJoinClicked()
{
    if (!acceptInput())
        return;

    QString chan = selectedChannel();
    if (chan.isEmpty())
        return;

    m_transitioning = true;
    emit joinRequested(chan);
}
