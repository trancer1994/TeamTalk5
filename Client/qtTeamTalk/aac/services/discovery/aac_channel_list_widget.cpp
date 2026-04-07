#include "aac_channel_list_widget.h"

#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

AACChannelListWidget::AACChannelListWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void AACChannelListWidget::setupUi()
{
    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);

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
    m_list->clear();
    m_list->addItems(channels);
}

QString AACChannelListWidget::selectedChannel() const
{
    auto* item = m_list->currentItem();
    return item ? item->text() : QString();
}

void AACChannelListWidget::onJoinClicked()
{
    QString chan = selectedChannel();
    if (chan.isEmpty())
        return;
    emit joinRequested(chan);
}
