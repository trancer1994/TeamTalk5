#include "aac_server_discovery.h"

#include <QHostAddress>
#include <QNetworkDatagram>

AACServerDiscovery::AACServerDiscovery(QObject* parent)
    : QObject(parent)
{
    // Window timer: fires once per discovery window
    m_windowTimer.setSingleShot(true);
    connect(&m_windowTimer, &QTimer::timeout,
            this, &AACServerDiscovery::endWindow);

    // Incremental timer: fires multiple times inside the window
    m_incrementalTimer.setInterval(m_incrementalIntervalMs);
    connect(&m_incrementalTimer, &QTimer::timeout,
            this, &AACServerDiscovery::performIncremental);

    // UDP socket (lazy-bound)
    m_udpSocket = new QUdpSocket(this);
    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &AACServerDiscovery::onUdpReadyRead);
}

void AACServerDiscovery::setDiscoveryWindowMs(int ms)
{
    if (ms <= 0)
        return;

    m_discoveryWindowMs = ms;
}

void AACServerDiscovery::startDiscovery()
{
    if (m_isScanning)
        return;

    clearWindowBuffer();
    beginWindow();
}

void AACServerDiscovery::scanAgain()
{
    stopDiscovery();
    clearWindowBuffer();
    beginWindow();
}

void AACServerDiscovery::stopDiscovery()
{
    if (!m_isScanning)
        return;

    m_windowTimer.stop();
    m_incrementalTimer.stop();
    setScanning(false);
}

void AACServerDiscovery::beginWindow()
{
    setScanning(true);

    ensureSocketBound();

    // Start incremental scanning (re-send probe during window)
    m_incrementalTimer.start();

    // Start the discovery window
    m_windowTimer.start(m_discoveryWindowMs);

    // Initial probe
    sendProbe();
}

void AACServerDiscovery::endWindow()
{
    // Stop incremental scanning
    m_incrementalTimer.stop();

    // Finalise results
    m_model.setServers(m_windowBuffer);

    updateLastUpdated();
    emit serversUpdated(m_windowBuffer);

    setScanning(false);
}

void AACServerDiscovery::performIncremental()
{
    // Re-send probe during the discovery window to catch late servers
    if (!m_isScanning)
        return;

    sendProbe();
}

void AACServerDiscovery::ensureSocketBound()
{
    if (!m_udpSocket)
        return;

    if (m_udpSocket->state() == QAbstractSocket::BoundState)
        return;

    // Bind to any available port on all interfaces, allow shared/broadcast
    m_udpSocket->bind(QHostAddress::AnyIPv4, 0,
                      QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
}

void AACServerDiscovery::sendProbe()
{
    if (!m_udpSocket)
        return;

    QHostAddress broadcast = QHostAddress::Broadcast;
    m_udpSocket->writeDatagram(m_probePayload, broadcast, m_discoveryPort);
}

void AACServerDiscovery::onUdpReadyRead()
{
    if (!m_isScanning)
    {
        // Ignore late packets outside the discovery window
        while (m_udpSocket->hasPendingDatagrams())
            m_udpSocket->readDatagram(nullptr, 0);
        return;
    }

    QVector<ServerInfo> partial;

    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        const QByteArray data = datagram.data().trimmed();
        if (data.isEmpty())
            continue;

        // Expected format: TT_SERVER <host> <port> <name...>
        const QList<QByteArray> parts = data.split(' ');
        if (parts.size() < 4)
            continue;

        if (!parts[0].startsWith("TT_SERVER"))
            continue;

        QString host = QString::fromUtf8(parts[1]);
        bool ok = false;
        int port = QString::fromUtf8(parts[2]).toInt(&ok);
        if (!ok || port <= 0)
            continue;

        // Name may contain spaces; join remaining parts
        QByteArray nameBytes;
        for (int i = 3; i < parts.size(); ++i) {
            if (!nameBytes.isEmpty())
                nameBytes.append(' ');
            nameBytes.append(parts[i]);
        }
        QString name = QString::fromUtf8(nameBytes);

        // If host is empty, fall back to sender address
        if (host.isEmpty())
            host = datagram.senderAddress().toString();

        ServerInfo info;
        info.name = name;
        info.host = host;
        info.port = port;

        partial.append(info);
    }

    if (!partial.isEmpty())
        mergeIncremental(partial);
}

void AACServerDiscovery::mergeIncremental(const QVector<ServerInfo>& partial)
{
    for (const auto& p : partial) {
        bool exists = false;
        for (const auto& existing : m_windowBuffer) {
            if (existing.host == p.host && existing.port == p.port) {
                exists = true;
                break;
            }
        }
        if (!exists)
            m_windowBuffer.append(p);
    }
}

void AACServerDiscovery::clearWindowBuffer()
{
    m_windowBuffer.clear();
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
