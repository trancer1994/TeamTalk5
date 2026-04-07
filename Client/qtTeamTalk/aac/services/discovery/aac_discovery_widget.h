#pragma once

#include <QWidget>
#include <QPointer>

class QListView;
class QPushButton;
class QLabel;

class ServerService;
class AACServerDiscovery;
class AACServerDiscoveryModel;
struct ServerInfo;

class AACDiscoveryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AACDiscoveryWidget(ServerService* service,
                                QWidget* parent = nullptr);

signals:
    void serverChosen(const ServerInfo& info);
    void backRequested();

private slots:
    void onStartDiscovery();
    void onRefresh();
    void onSelect();
    void onSelectionChanged(const ServerInfo& info);
    void onStateChanged(int state);
    void onError(const QString& message);

private:
    void setupUi();
    void wireSignals();

private:
    QPointer<ServerService> m_service;
    AACServerDiscovery* m_discovery = nullptr;
    AACServerDiscoveryModel* m_model = nullptr;

    QListView* m_listView = nullptr;
    QPushButton* m_startButton = nullptr;
    QPushButton* m_refreshButton = nullptr;
    QPushButton* m_selectButton = nullptr;
    QPushButton* m_backButton = nullptr;
    QLabel* m_statusLabel = nullptr;

    ServerInfo* m_currentSelection = nullptr;
};
