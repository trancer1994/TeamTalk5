#pragma once

#include <QString>
#include <QDateTime>

enum class ServerSource {
    Local,
    Latest,
    Public,
    JoinCode,
    DeepLink,
    QRCode,
    ImportedTT
};

struct ServerInfo {
    QString id;              // stable ID for merging (e.g. host:tcp:udp:username:channel)
    QString name;

    QString host;
    int tcpPort = 10333;
    int udpPort = 10333;
    bool encrypted = false;

    QString username;
    QString password;
    QString nickname;
    QString statusMessage;

    QString channel;
    QString channelPassword;

    QString joinCode;

    // Public server metadata
    QString country;
    int userCount = 0;
    QString motd;
    QString serverName;      // public listing name, if any

    ServerSource source = ServerSource::Local;
    QDateTime lastSeen;      // for cache / latest hosts

    bool isValid() const {
        return !host.isEmpty() && tcpPort > 0 && udpPort > 0;
    }

    static QString makeId(const QString& host,
                          int tcpPort,
                          int udpPort,
                          const QString& username,
                          const QString& channel)
    {
        return host.toLower() + QLatin1Char(':')
             + QString::number(tcpPort) + QLatin1Char(':')
             + QString::number(udpPort) + QLatin1Char(':')
             + username.toLower() + QLatin1Char(':')
             + channel.toLower();
    }

    void updateId() {
        id = makeId(host, tcpPort, udpPort, username, channel);
    }
};
