#include "aac_discovery_widget.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QShowEvent>
#include <QHideEvent>

static const int kDefaultDwellTimeMs = 1000;

AACDiscoveryWidget::AACDiscoveryWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setMouseTracking(true);

    layout->addWidget(m_list);
    setLayout(layout);

    connect(&m_discovery, &AACServerDiscovery::serversUpdated,
            this, &AACDiscoveryWidget::onServersUpdated);

    connect(m_discovery.model(), &AACServerDiscoveryModel::highlightChanged,
            this, &AACDiscoveryWidget::onHighlightChanged);

    connect(m_discovery.model(), &AACServerDiscoveryModel::selectionChanged,
            this, &AACDiscoveryWidget::onSelectionChanged);

    connect(m_list, &QListWidget::itemEntered,
            this, &AACDiscoveryWidget::onItemEntered);

    m_dwellTimer.setInterval(kDefaultDwellTimeMs);
    m_dwellTimer.setSingleShot(true);
    connect(&m_dwellTimer, &QTimer::timeout,
            this, &AACDiscoveryWidget::commitDwell);
}

void AACDiscoveryWidget::setDwellTimeMs(int ms)
{
    if (ms <= 0)
        return;
    m_dwellTimer.setInterval(ms);
}

void AACDiscoveryWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    m_discovery.startDiscovery();
}

void AACDiscoveryWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    m_discovery.stopDiscovery();
    cancelDwell();
}

void AACDiscoveryWidget::onServersUpdated(const QVector<ServerInfo>& servers)
{
    restoreHighlightIfPossible(servers);
    rebuildList(servers);
}

void AACDiscoveryWidget::restoreHighlightIfPossible(const QVector<ServerInfo>& servers)
{
    Q_UNUSED(servers);

    if (m_lastHighlightKey.isEmpty())
        return;

    int row = m_discovery.model()->rowForKey(m_lastHighlightKey);
    if (row >= 0)
        m_discovery.model()->setHighlight(row);
}

void AACDiscoveryWidget::rebuildList(const QVector<ServerInfo>& servers)
{
    m_list->clear();

    for (const ServerInfo& s : servers) {
        QListWidgetItem* item = new QListWidgetItem(
            QString("%1 (%2:%3)").arg(s.name).arg(s.host).arg(s.port));
        m_list->addItem(item);
    }

    int row = m_discovery.model()->highlightIndex();
    applyHighlight(row);
}

void AACDiscoveryWidget::onHighlightChanged(int row)
{
    m_lastHighlightKey = m_discovery.model()->keyForRow(row);
    applyHighlight(row);
}

void AACDiscoveryWidget::applyHighlight(int row)
{
    for (int i = 0; i < m_list->count(); ++i) {
        QListWidgetItem* item = m_list->item(i);
        item->setSelected(i == row);
    }
}

void AACDiscoveryWidget::onSelectionChanged(const ServerInfo& info)
{
    emit serverChosen(info);
}

void AACDiscoveryWidget::onItemEntered(QListWidgetItem* item)
{
    if (m_committingSelection)
        return;

    int row = m_list->row(item);
    m_discovery.model()->setHighlight(row);

    m_pendingRow = row;
    m_dwellTimer.start();
}

void AACDiscoveryWidget::cancelDwell()
{
    m_dwellTimer.stop();
    m_pendingRow = -1;
}

void AACDiscoveryWidget::commitDwell()
{
    if (m_pendingRow < 0)
        return;

    m_committingSelection = true;
    m_discovery.stopDiscovery();

    m_discovery.model()->setSelected(m_pendingRow);

    m_pendingRow = -1;
    m_committingSelection = false;
}
