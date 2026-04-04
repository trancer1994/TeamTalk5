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

QVariantList numbers() const;
QVariantList places() const;
QVariantList coreSymbols() const;
bool loadPublicCache(QByteArray& xml, QDateTime& timestamp) const;
    void savePublicCache(const QByteArray& xml, const QDateTime& timestamp);

private:
    QString settingsGroupPersonal() const { return QStringLiteral("aac/personal_servers"); }
    QString settingsGroupLatest()   const { return QStringLiteral("aac/latest_hosts"); }
    QString cacheFilePath() const;
    QString cacheMetaPath() const;
QString aacDataDir() const;
QString numbersPath() const;
QString placesPath() const;
QString coreSymbolsPath() const;

 mutable QVariantList m_numbersCache;
mutable QVariantList m_placesCache;
mutable QVariantList m_coreSymbolsCache;

mutable bool m_numbersLoaded = false;
mutable bool m_placesLoaded = false;
mutable bool m_coreLoaded = false;

ServerInfo fromSettings(const QString& prefix) const;
    void toSettings(const QString& prefix, const ServerInfo& info) const;
};
