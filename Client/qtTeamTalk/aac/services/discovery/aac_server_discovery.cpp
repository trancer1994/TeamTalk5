#include "aac_server_discovery.h"
#include <QHostInfo>

AACServerDiscovery::AACServerDiscovery(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(m_discoveryIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &AACServerDiscovery::performDiscovery);
}

void AACServerDiscovery::setDiscoveryIntervalMs(int ms)
{
    if (ms <= 0)
        return;

    m_discoveryIntervalMs = ms;
    m_timer.setInterval(m_discoveryIntervalMs);
}

void AACServerDiscovery::startDiscovery()
{
    if (m_isScanning)
        return;

    setScanning(true);
    m_timer.start();
    performDiscovery();
}

void AACServerDiscovery::stopDiscovery()
{
    if (!m_isScanning)
        return;

    m_timer.stop();
    setScanning(false);
}

void AACServerDiscovery::setScanning(bool scanning)
{
    if (m_isScanning == scanning)
        return;

    m_isScanning = scanning;
    emit isScanningChanged(m_isScanning);
}

void AACServerDiscovery::updateLastUpdated()
{
    m_lastUpdated = QDateTime::currentDateTime();
    emit lastUpdatedChanged(m_lastUpdated);
}

void AACServerDiscovery::performDiscovery()
{
    QVector<ServerInfo> servers;

    ServerInfo s1;
    s1.name = "Local Server";
    s1.host = "127.0.0.1";
    s1.port = 10333;
    servers.append(s1);

    ServerInfo s2;
    s2.name = "Demo Server";
    s2.host = "192.168.1.50";
    s2.port = 10333;
    servers.append(s2);

    m_model.setServers(servers);
    updateLastUpdated();
    emit serversUpdated(servers);
}
