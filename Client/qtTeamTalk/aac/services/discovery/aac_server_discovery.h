#pragma once

#include <QObject>
#include <QList>
#include <QTimer>
#include <QDateTime>

#include "Client/qtTeamTalk/aac/models/serverinfo.h"
#include "Client/qtTeamTalk/aac/storage/aacstorage.h"

class ServerService; // forward declaration

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

    // Actions
public slots:
    void startDiscovery();
    void stopDiscovery();
    void selectServer(const ServerInfo& info);
    void restart();

signals:
    void stateChanged(AACServerDiscovery::State newState);
    void serverFound(const ServerInfo& info);
    void serverListCleared();
    void resolved(const ServerInfo& info);
    void errorOccurred(const QString& message);

private slots:
    void onLANDiscoveryTick();

private:
    void setState(State s);
    void emitCachedServers();
    void beginLANDiscovery();
    void endLANDiscovery();

private:
    State m_state = State::Idle;

    ServerService* m_service = nullptr;
    AACStorage m_storage;

    QList<ServerInfo> m_seen;
    QTimer m_lanTimer;
};
