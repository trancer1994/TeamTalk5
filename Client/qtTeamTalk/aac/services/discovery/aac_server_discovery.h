#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include "aac_server_discovery_model.h"

class AACServerDiscovery : public QObject
{
    Q_OBJECT

public:
    explicit AACServerDiscovery(QObject* parent = nullptr);

    AACServerDiscoveryModel* model() { return &m_model; }

    void startDiscovery();
    void stopDiscovery();

    bool isScanning() const { return m_isScanning; }
    QDateTime lastUpdated() const { return m_lastUpdated; }

    int discoveryIntervalMs() const { return m_discoveryIntervalMs; }
    void setDiscoveryIntervalMs(int ms);

signals:
    void serversUpdated(const QVector<ServerInfo>& servers);
    void isScanningChanged(bool scanning);
    void lastUpdatedChanged(const QDateTime& dt);

private slots:
    void performDiscovery();

private:
    void setScanning(bool scanning);
    void updateLastUpdated();

    AACServerDiscoveryModel m_model;
    QTimer m_timer;

    bool m_isScanning = false;
    QDateTime m_lastUpdated;

    int m_discoveryIntervalMs = 3000;
};
