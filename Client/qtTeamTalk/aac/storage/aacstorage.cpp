#include "aacstorage.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDataStream>

AACStorage::AACStorage() = default;

ServerInfo AACStorage::fromSettings(const QString& prefix) const
{
    QSettings s;
    ServerInfo info;
    info.name          = s.value(prefix + "/name").toString();
    info.host          = s.value(prefix + "/host").toString();
    info.tcpPort       = s.value(prefix + "/tcpPort", 10333).toInt();
    info.udpPort       = s.value(prefix + "/udpPort", 10333).toInt();
    info.encrypted     = s.value(prefix + "/encrypted", false).toBool();
    info.username      = s.value(prefix + "/username").toString();
    info.password      = s.value(prefix + "/password").toString();
    info.nickname      = s.value(prefix + "/nickname").toString();
    info.statusMessage = s.value(prefix + "/statusMessage").toString();
    info.channel       = s.value(prefix + "/channel").toString();
    info.channelPassword = s.value(prefix + "/channelPassword").toString();
    info.joinCode      = s.value(prefix + "/joinCode").toString();
    info.country       = s.value(prefix + "/country").toString();
    info.userCount     = s.value(prefix + "/userCount", 0).toInt();
    info.motd          = s.value(prefix + "/motd").toString();
    info.serverName    = s.value(prefix + "/serverName").toString();
    info.source        = static_cast<ServerSource>(s.value(prefix + "/source",
                                                           static_cast<int>(ServerSource::Local)).toInt());
    info.lastSeen      = s.value(prefix + "/lastSeen").toDateTime();
    info.updateId();
    return info;
}

void AACStorage::toSettings(const QString& prefix, const ServerInfo& info) const
{
    QSettings s;
    s.setValue(prefix + "/name", info.name);
    s.setValue(prefix + "/host", info.host);
    s.setValue(prefix + "/tcpPort", info.tcpPort);
    s.setValue(prefix + "/udpPort", info.udpPort);
    s.setValue(prefix + "/encrypted", info.encrypted);
    s.setValue(prefix + "/username", info.username);
    s.setValue(prefix + "/password", info.password);
    s.setValue(prefix + "/nickname", info.nickname);
    s.setValue(prefix + "/statusMessage", info.statusMessage);
    s.setValue(prefix + "/channel", info.channel);
    s.setValue(prefix + "/channelPassword", info.channelPassword);
    s.setValue(prefix + "/joinCode", info.joinCode);
    s.setValue(prefix + "/country", info.country);
    s.setValue(prefix + "/userCount", info.userCount);
    s.setValue(prefix + "/motd", info.motd);
    s.setValue(prefix + "/serverName", info.serverName);
    s.setValue(prefix + "/source", static_cast<int>(info.source));
    s.setValue(prefix + "/lastSeen", info.lastSeen);
}

QList<ServerInfo> AACStorage::loadPersonalServers() const
{
    QSettings s;
    QList<ServerInfo> result;
    s.beginGroup(settingsGroupPersonal());
    int count = s.value("count", 0).toInt();
    for (int i = 0; i < count; ++i) {
        result.append(fromSettings(QString::number(i)));
    }
    s.endGroup();
    return result;
}

void AACStorage::savePersonalServers(const QList<ServerInfo>& servers)
{
    QSettings s;
    s.beginGroup(settingsGroupPersonal());
    s.remove(QString()); // clear group
    s.setValue("count", servers.size());
    for (int i = 0; i < servers.size(); ++i) {
        toSettings(QString::number(i), servers[i]);
    }
    s.endGroup();
}

QList<ServerInfo> AACStorage::loadLatestHosts() const
{
    QSettings s;
    QList<ServerInfo> result;
    s.beginGroup(settingsGroupLatest());
    int count = s.value("count", 0).toInt();
    for (int i = 0; i < count; ++i) {
        result.append(fromSettings(QString::number(i)));
    }
    s.endGroup();
    return result;
}

void AACStorage::saveLatestHosts(const QList<ServerInfo>& hosts)
{
    QSettings s;
    s.beginGroup(settingsGroupLatest());
    s.remove(QString());
    s.setValue("count", hosts.size());
    for (int i = 0; i < hosts.size(); ++i) {
        toSettings(QString::number(i), hosts[i]);
    }
    s.endGroup();
}

QString AACStorage::cacheFilePath() const
{
    const auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir d(dir);
    d.mkpath(QStringLiteral("."));
    return d.filePath(QStringLiteral("aac_serverlist_cache.xml"));
}

QString AACStorage::cacheMetaPath() const
{
    const auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir d(dir);
    d.mkpath(QStringLiteral("."));
    return d.filePath(QStringLiteral("aac_serverlist_cache.meta"));
}

bool AACStorage::loadPublicCache(QByteArray& xml, QDateTime& timestamp) const
{
    QFile f(cacheFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return false;
    xml = f.readAll();
    f.close();

    QFile meta(cacheMetaPath());
    if (!meta.open(QIODevice::ReadOnly))
        return false;
    QDataStream in(&meta);
    in >> timestamp;
    return true;
}

void AACStorage::savePublicCache(const QByteArray& xml, const QDateTime& timestamp)
{
    QFile f(cacheFilePath());
    if (f.open(QIODevice::WriteOnly)) {
        f.write(xml);
        f.close();
    }

    QFile meta(cacheMetaPath());
    if (meta.open(QIODevice::WriteOnly)) {
        QDataStream out(&meta);
        out << timestamp;
        meta.close();
    }
}
