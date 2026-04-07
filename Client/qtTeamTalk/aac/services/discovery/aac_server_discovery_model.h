#pragma once

#include <QAbstractListModel>
#include "Client/qtTeamTalk/aac/models/serverinfo.h"

class AACServerDiscoveryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        HostRole,
        TcpPortRole,
        UdpPortRole,
        SourceRole,
        SelectedRole
    };

    explicit AACServerDiscoveryModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void addServer(const ServerInfo& info);
    void clear();
    void toggleSelection(int row);
    ServerInfo selectedServer() const;

signals:
    void selectionChanged(const ServerInfo& info);

private:
    QList<ServerInfo> m_items;
    int m_selected = -1;
};
