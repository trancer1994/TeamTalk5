#include "aac_server_discovery.h"
#include "serverservice.h"

#include <QDomDocument>
#include <QHostAddress>
#include <QNetworkInterface>

AACServerDiscovery::AACServerDiscovery(ServerService* service,
                                       QObject* parent)
    : QObject(parent)
    , m_service(service)
{
}

void AACServerDiscovery::setState(State s)
{
    if (m_state == s)
        return;

    m_state = s;
    emit stateChanged(s);
}

void AACServerDiscovery::startDiscovery()
{
    if (!m_service) {
        emit errorOccurred("ServerService unavailable");
        return;
    }

    m_seen.clear();
    emit serverListCleared();

    setState(State::Discovering);

    // 1) Cached servers first (instant feedback)
    emitCachedServers();

    // 2) Public servers (async)
    connect(m_service, &ServerService::publicServersUpdated,
            this, [this](const QList<ServerInfo>& list) {
        for (const auto& s : list) {
            if (!m_seen.contains(s)) {
                m_seen.append(s);
                emit serverFound(s);
            }
        }
    });

    m_service->fetchPublicServers(true);

    // 3) LAN discovery
    beginLANDiscovery();
}

void AACServerDiscovery::stopDiscovery()
{
    endLANDiscovery();
    setState(State::Idle);
}

void AACServerDiscovery::restart()
{
    stopDiscovery();
    startDiscovery();
}

void AACServerDiscovery::selectServer(const ServerInfo& info)
{
    if (m_state != State::Discovering)
        return;

    setState(State::Resolved);
    emit resolved(info);
}

void AACServerDiscovery::emitCachedServers()
{
    QList<ServerInfo> cached = m_storage.loadLatestHosts();
    for (auto s : cached) {
        s.source = ServerSource::Cached;
        s.lastSeen = QDateTime::currentDateTimeUtc();
        s.updateId();

        m_seen.append(s);
        emit serverFound(s);
    }
}

void AACServerDiscovery::beginLANDiscovery()
{
    if (!m_udp) {
        m_udp = new QUdpSocket(this);

        connect(m_udp, &QUdpSocket::readyRead,
                this, [this] {
            while (m_udp->hasPendingDatagrams()) {
                QByteArray datagram;
                datagram.resize(int(m_udp->pendingDatagramSize()));
                m_udp->readDatagram(datagram.data(), datagram.size());
                handleLANResponse(datagram);
            }
        });
    }

    // Broadcast "TTLAN" to all interfaces
    QByteArray request("TTLAN");

    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp))
            continue;
        if (!(iface.flags() & QNetworkInterface::IsRunning))
            continue;
        if (iface.flags() & QNetworkInterface::IsLoopBack)
            continue;

        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            QHostAddress broadcast = entry.broadcast();
            if (!broadcast.isNull()) {
                m_udp->writeDatagram(request, broadcast, 10333);
            }
        }
    }
}

void AACServerDiscovery::endLANDiscovery()
{
    if (m_udp) {
        m_udp->close();
        m_udp->deleteLater();
        m_udp = nullptr;
    }
}

void AACServerDiscovery::handleLANResponse(const QByteArray& datagram)
{
    QDomDocument doc;
    if (!doc.setContent(datagram))
        return;

    QDomElement root = doc.documentElement();
    QDomElement hostElem = root.firstChildElement("host");
    if (hostElem.isNull())
        return;

    ServerInfo info;
    if (!m_service->parseServerElement(hostElem, info))
        return;

    info.source = ServerSource::LAN;
    info.lastSeen = QDateTime::currentDateTimeUtc();
    info.updateId();

    if (!m_seen.contains(info)) {
        m_seen.append(info);
        emit serverFound(info);
    }
}
