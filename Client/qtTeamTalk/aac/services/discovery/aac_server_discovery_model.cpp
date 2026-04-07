#include "aac_server_discovery_model.h"

AACServerDiscoveryModel::AACServerDiscoveryModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int AACServerDiscoveryModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QVariant AACServerDiscoveryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const auto& s = m_items[index.row()];

    switch (role) {
    case NameRole: return s.name;
    case HostRole: return s.host;
    case TcpPortRole: return s.tcpPort;
    case UdpPortRole: return s.udpPort;
    case SourceRole: return static_cast<int>(s.source);
    case SelectedRole: return index.row() == m_selected;
    }
    return {};
}

QHash<int, QByteArray> AACServerDiscoveryModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { HostRole, "host" },
        { TcpPortRole, "tcpPort" },
        { UdpPortRole, "udpPort" },
        { SourceRole, "source" },
        { SelectedRole, "selected" }
    };
}

void AACServerDiscoveryModel::addServer(const ServerInfo& info)
{
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(info);
    endInsertRows();
}

void AACServerDiscoveryModel::clear()
{
    beginResetModel();
    m_items.clear();
    m_selected = -1;
    endResetModel();
}

void AACServerDiscoveryModel::toggleSelection(int row)
{
    if (row < 0 || row >= m_items.size())
        return;

    int old = m_selected;
    m_selected = (m_selected == row ? -1 : row);

    if (old >= 0)
        emit dataChanged(index(old), index(old), { SelectedRole });
    if (m_selected >= 0)
        emit dataChanged(index(m_selected), index(m_selected), { SelectedRole });

    if (m_selected >= 0)
        emit selectionChanged(m_items[m_selected]);
}

ServerInfo AACServerDiscoveryModel::selectedServer() const
{
    if (m_selected < 0 || m_selected >= m_items.size())
        return {};
    return m_items[m_selected];
}
