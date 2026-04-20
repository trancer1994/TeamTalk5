#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QUdpSocket>

#include "aac_server_discovery_model.h"

class AACServerDiscovery : public QObject
{
    Q_OBJECT

public:
    explicit AACServerDiscovery(QObject* parent = nullptr);

    AACServerDiscoveryModel* model() { return &m_model; }

    // High-level control
    void startDiscovery();       // triggers a full discovery window
    void scanAgain();            // user-triggered re-scan
    void stopDiscovery();        // stops timers, freezes state

    bool isScanning() const { return m_isScanning; }
    QDateTime lastUpdated() const { return m_lastUpdated; }

    int discoveryWindowMs() const { return m_discoveryWindowMs; }
    void setDiscoveryWindowMs(int ms);

signals:
    // Emitted once per discovery window when the final list is stable
    void serversUpdated(const QVector<ServerInfo>& servers);

    // AAC-friendly state signalling
    void isScanningChanged(bool scanning);
    void lastUpdatedChanged(const QDateTime& dt);

private slots:
    void beginWindow();          // start of a discovery window
    void endWindow();            // end of a discovery window
    void performIncremental();   // incremental LAN scan tick
    void onUdpReadyRead();       // handle incoming UDP replies

private:
    void setScanning(bool scanning);
    void updateLastUpdated();

    // Internal helpers
    void clearWindowBuffer();
    void mergeIncremental(const QVector<ServerInfo>& partial);

    void ensureSocketBound();
    void sendProbe();

    // Internal state
    AACServerDiscoveryModel m_model;

    QTimer m_windowTimer;        // controls the discovery window
    QTimer m_incrementalTimer;   // incremental LAN scan ticks

    bool m_isScanning = false;
    QDateTime m_lastUpdated;

    int m_discoveryWindowMs = 1800;    // 1.8s stable window
    int m_incrementalIntervalMs = 300; // 300ms incremental ticks

    QVector<ServerInfo> m_windowBuffer; // accumulates results during window

    QUdpSocket* m_udpSocket = nullptr;
    quint16 m_discoveryPort = 10333;
    QByteArray m_probePayload = QByteArrayLiteral("DISCOVER_TEAMTALK");
};
