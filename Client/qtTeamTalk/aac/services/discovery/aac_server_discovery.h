#pragma once

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QUdpSocket>

#include "Client/qtTeamTalk/aac/models/serverinfo.h"
#include "Client/qtTeamTalk/aac/storage/aacstorage.h"

class ServerService;

class AACServerDiscovery : public QObject
{
    Q_OBJECT
public:
    enum class State {
        Idle,
        Discovering,
        Resolved
    };
    Q_ENUM(State)

    explicit AACServerDiscovery(ServerService* service,
                                QObject* parent = nullptr);

    State state() const { return m_state; }

public slots:
    void startDiscovery();
    void stopDiscovery();
    void restart();
    void selectServer(const ServerInfo& info);

signals:
    void stateChanged(AACServerDiscovery::State newState);
    void serverFound(const ServerInfo& info);
    void serverListCleared();
    void resolved(const ServerInfo& info);
    void errorOccurred(const QString& message);

private:
    void setState(State s);
    void emitCachedServers();
    void beginLANDiscovery();
    void endLANDiscovery();
    void handleLANResponse(const QByteArray& datagram);

private:
    State m_state = State::Idle;

    ServerService* m_service = nullptr;
    AACStorage m_storage;

    QList<ServerInfo> m_seen;
    QUdpSocket* m_udp = nullptr;
};
