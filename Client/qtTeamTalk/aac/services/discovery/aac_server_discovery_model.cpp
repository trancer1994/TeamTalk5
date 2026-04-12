#include "aac_server_discovery_model.h"

AACServerDiscoveryModel::AACServerDiscoveryModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int AACServerDiscoveryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_servers.size();
}

QVariant AACServerDiscoveryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_servers.size())
        return QVariant();

    const ServerInfo& info = m_servers.at(index.row());

    switch (role) {
    case NameRole: return info.name;
    case HostRole: return info.host;
    case PortRole: return info.port;
    case KeyRole:  return info.key();
    default:       return QVariant();
    }
}

QHash<int, QByteArray> AACServerDiscoveryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[HostRole] = "host";
    roles[PortRole] = "port";
    roles[KeyRole]  = "key";
    return roles;
}

void AACServerDiscoveryModel::setServers(const QVector<ServerInfo>& servers)
{
    beginResetModel();
    m_servers = servers;
    m_highlightIndex = -1;
    endResetModel();
}

int AACServerDiscoveryModel::rowForKey(const QString& key) const
{
    if (key.isEmpty())
        return -1;

    for (int i = 0; i < m_servers.size(); ++i) {
        if (m_servers[i].key() == key)
            return i;
    }
    return -1;
}

QString AACServerDiscoveryModel::keyForRow(int row) const
{
    if (row < 0 || row >= m_servers.size())
        return QString();
    return m_servers[row].key();
}

void AACServerDiscoveryModel::setHighlight(int row)
{
    if (row == m_highlightIndex)
        return;

    if (row < -1 || row >= m_servers.size())
        return;

    m_highlightIndex = row;
    emit highlightChanged(row);
}

void AACServerDiscoveryModel::setSelected(int row)
{
    if (row < 0 || row >= m_servers.size())
        return;

    emit selectionChanged(m_servers[row]);
}
