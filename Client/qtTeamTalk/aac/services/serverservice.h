#pragma once

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QDomElement>
#include "Client/qtTeamTalk/aac/models/serverinfo.h"
#include "Client/qtTeamTalk/aac/storage/aacstorage.h"

class ServerService : public QObject
{
    Q_OBJECT
public:
    explicit ServerService(QObject* parent = nullptr);

    // Public server list
    void fetchPublicServers(bool useCache = true);

    // Join codes
    void resolveJoinCode(const QString& code);

    // Deep links (teamtalk://join?... style)
    ServerInfo parseDeepLink(const QString& url) const;
    QString   toDeepLink(const ServerInfo& info) const;

    // QR payloads (we treat them as either join code or deep link)
    ServerInfo resolveQRPayload(const QString& payload);

    // .tt import/export
    QList<ServerInfo> importTTFile(const QString& path);
    bool exportTTFileSingle(const QString& path, const QList<ServerInfo>& servers);
    bool exportTTFileMultiple(const QString& dir, const QList<ServerInfo>& servers);

    // Personal servers
    QList<ServerInfo> loadPersonalServers() const;
    void savePersonalServers(const QList<ServerInfo>& servers);

    // Latest hosts
    QList<ServerInfo> loadLatestHosts() const;
    void saveLatestHosts(const QList<ServerInfo>& hosts);

signals:
    void publicServersUpdated(const QList<ServerInfo>& servers);
    void joinCodeResolved(const ServerInfo& server);
    void joinCodeFailed(const QString& reason);

private slots:
    void onPublicServersReply(QNetworkReply* reply);
    void onJoinCodeReply(QNetworkReply* reply);

private:
    AACStorage m_storage;
    QNetworkAccessManager* m_publicManager = nullptr;
    QNetworkAccessManager* m_joinManager   = nullptr;

    QList<ServerInfo> m_publicServers;

    bool parseServerElement(const QDomElement& e, ServerInfo& out) const;
    QList<ServerInfo> parsePublicXML(const QByteArray& data) const;

    QList<ServerInfo> mergeServerLists(const QList<ServerInfo>& a,
                                       const QList<ServerInfo>& b) const;
};
