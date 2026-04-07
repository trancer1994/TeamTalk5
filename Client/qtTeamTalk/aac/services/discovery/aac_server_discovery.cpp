#include "aac_server_discovery.h"
#include "serverservice.h"

#include <QDebug>

AACServerDiscovery::AACServerDiscovery(ServerService* service,
                                       QObject* parent)
    : QObject(parent)
    , m_service(service)
{
    connect(&m_lanTimer, &QTimer::timeout,
            this, &AACServerDiscovery::onLANDiscoveryTick);
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

    // 1) Emit cached servers immediately
    emitCachedServers();

    // 2) Fetch public servers (async)
    m_service->fetchPublicServers(true);
    connect(m_service, &ServerService::publicServersUpdated,
            this, [this](const QList<ServerInfo>& list) {
                for (const auto& s : list) {
                    if (!m_seen.contains(s)) {
                        m_seen.append(s);
                        emit serverFound(s);
                    }
                }
            });

    // 3) Begin LAN discovery
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
    for (auto& s : cached) {
        s.source = ServerSource::Cached;
        s.updateId();
        m_seen.append(s);
        emit serverFound(s);
    }
}

void AACServerDiscovery::beginLANDiscovery()
{
    // LAN discovery is simulated here; you will replace this
    // with TeamTalk's UDP broadcast listener.
    m_lanTimer.start(1500);
}

void AACServerDiscovery::endLANDiscovery()
{
    m_lanTimer.stop();
}

void AACServerDiscovery::onLANDiscoveryTick()
{
    // Placeholder LAN discovery result
    ServerInfo info;
    info.host = "192.168.1.100";
    info.tcpPort = 10333;
    info.udpPort = 10333;
    info.name = "LAN Server";
    info.source = ServerSource::LAN;
    info.lastSeen = QDateTime::currentDateTimeUtc();
    info.updateId();

    if (!m_seen.contains(info)) {
        m_seen.append(info);
        emit serverFound(info);
    }
}
