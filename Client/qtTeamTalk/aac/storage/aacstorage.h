#pragma once

#include <QList>
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include "Client/qtTeamTalk/aac/models/serverinfo.h"

class AACStorage
{
public:
    AACStorage();

    QList<ServerInfo> loadPersonalServers() const;
    void savePersonalServers(const QList<ServerInfo>& servers);

    QList<ServerInfo> loadLatestHosts() const;
    void saveLatestHosts(const QList<ServerInfo>& hosts);

    bool loadPublicCache(QByteArray& xml, QDateTime& timestamp) const;
    void savePublicCache(const QByteArray& xml, const QDateTime& timestamp);

private:
    QString settingsGroupPersonal() const { return QStringLiteral("aac/personal_servers"); }
    QString settingsGroupLatest()   const { return QStringLiteral("aac/latest_hosts"); }
    QString cacheFilePath() const;
    QString cacheMetaPath() const;

    ServerInfo fromSettings(const QString& prefix) const;
    void toSettings(const QString& prefix, const ServerInfo& info) const;
};
