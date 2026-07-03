#include "ChannelListScreen.h"
#include "AACKeyButton.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>

ChannelListScreen::ChannelListScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    auto* main = new QVBoxLayout(this);
    auto* title = new QLabel(tr("Channels"), this);
    main->addWidget(title);

    setLayout(main);
}
QString ChannelListScreen::contextualHelp() const
{
    return tr("ChannelList. "
               "Use the arrow keys to choose a channel. "
               "Press Enter to join. "
               "Press F5 to refresh. "
               "Press Escape to go back.");
}
void ChannelListScreen::setChannels(const QList<ChannelInfo>& channels)
{
    m_channels = channels;

    auto* main = qobject_cast<QVBoxLayout*>(layout());

    // Clear everything except title (index 0)
    while (main->count() > 1) {
        auto* item = main->takeAt(1);
        delete item->widget();
        delete item;
    }

    m_channelButtons.clear();

    // Build channel buttons
    for (const auto& ch : m_channels) {
        auto* btn = new AACKeyButton(aac(), ch.name, this);
        main->addWidget(btn);
        m_channelButtons.append(btn);

        connect(btn, &AACKeyButton::activated, this, [this, ch]() {
            emit channelChosen(ch.id);
        });
    }

    // Back + Refresh row
    auto* row = new QHBoxLayout();
    auto* backBtn = new AACKeyButton(aac(), tr("Back"), this);
    auto* refreshBtn = new AACKeyButton(aac(), tr("Refresh"), this);
    row->addWidget(backBtn);
    row->addWidget(refreshBtn);
    main->addLayout(row);

    connect(backBtn, &AACKeyButton::activated, this, [this]() {
        if (aac()->speechEngine())
            aac()->speechEngine()->speak(tr("Back"));
        emit backRequested();
    });

    connect(refreshBtn, &AACKeyButton::activated, this, [this]() {
        if (aac()->speechEngine())
            aac()->speechEngine()->speak(tr("Refreshing"));
        emit refreshRequested();
    });

    // AAC-native spoken announcement
    announceChannelCount(m_channels.size());
}

void ChannelListScreen::announceChannelCount(int count)
{
    if (!aac() || !aac()->speechEngine())
        return;

    QString msg;
    if (count == 0)
        msg = tr("No channels found");
    else if (count == 1)
        msg = tr("1 channel found");
    else
        msg = tr("%1 channels found").arg(count);

    aac()->speechEngine()->speak(msg);
}

void ChannelListScreen::keyPressEvent(QKeyEvent* e)
{
    // ESC → Back
    if (e->key() == Qt::Key_Escape) {
        emit backRequested();
        return;
    }

    // Home → first channel
    if (e->key() == Qt::Key_Home && !m_channelButtons.isEmpty()) {
        m_channelButtons.first()->setFocus();
        return;
    }

    // End → last channel
    if (e->key() == Qt::Key_End && !m_channelButtons.isEmpty()) {
        m_channelButtons.last()->setFocus();
        return;
    }

    AACScreenBase::keyPressEvent(e);
}
