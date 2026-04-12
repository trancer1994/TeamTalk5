#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct ServerInfo {
    QString name;
    QString host;
    int port = 0;

    QString key() const {
        return host + ":" + QString::number(port);
    }
};

class AACServerDiscoveryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        HostRole,
        PortRole,
        KeyRole
    };

    explicit AACServerDiscoveryModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setServers(const QVector<ServerInfo>& servers);

    int rowForKey(const QString& key) const;
    QString keyForRow(int row) const;

    int highlightIndex() const { return m_highlightIndex; }
    void setHighlight(int row);

    void setSelected(int row);

signals:
    void highlightChanged(int row);
    void selectionChanged(const ServerInfo& info);

private:
    QVector<ServerInfo> m_servers;
    int m_highlightIndex = -1;
};
