#pragma once

#include <QWidget>
#include <QTimer>
#include <QPointer>
#include "aac_server_discovery.h"

class QListWidget;
class QListWidgetItem;

class AACDiscoveryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AACDiscoveryWidget(QWidget* parent = nullptr);

    int dwellTimeMs() const { return m_dwellTimer.interval(); }
    void setDwellTimeMs(int ms);

signals:
    void serverChosen(const ServerInfo& info);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void onServersUpdated(const QVector<ServerInfo>& servers);
    void onHighlightChanged(int row);
    void onSelectionChanged(const ServerInfo& info);

    void onItemEntered(QListWidgetItem* item);
    void commitDwell();

private:
    void rebuildList(const QVector<ServerInfo>& servers);
    void applyHighlight(int row);
    void cancelDwell();

    void restoreHighlightIfPossible(const QVector<ServerInfo>& servers);

    AACServerDiscovery m_discovery;
    QListWidget* m_list = nullptr;

    QTimer m_dwellTimer;
    int m_pendingRow = -1;

    QString m_lastHighlightKey;
    bool m_committingSelection = false;
};
