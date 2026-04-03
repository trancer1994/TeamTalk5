#include "serverservice.h"

#include "appinfo.h"
#include "settings.h"
#include "utilxml.h"
#include "generatettfiledlg.h"
#include "common.h"

#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QFileDialog>
#include <QDomDocument>
#include <QDir>
#include <QUrlQuery>

extern NonDefaultSettings* ttSettings;

ServerService::ServerService(QObject* parent)
    : QObject(parent)
{
}

void ServerService::fetchPublicServers(bool useCache)
{
    // Try cache first
    if (useCache) {
        QByteArray xml;
        QDateTime ts;
        if (m_storage.loadPublicCache(xml, ts)) {
            // 5-minute cache window
            if (ts.secsTo(QDateTime::currentDateTimeUtc()) < 300) {
                auto parsed = parsePublicXML(xml);
                m_publicServers = parsed;
                emit publicServersUpdated(parsed);
                // still fall through to refresh if you want; here we don't
                return;
            }
        }
    }

    if (!m_publicManager)
        m_publicManager = new QNetworkAccessManager(this);

    bool officialservers   = true;
    bool publicservers     = true;
    bool unofficialservers = true;

    QUrl url(URL_FREESERVER(officialservers, publicservers, unofficialservers));
    connect(m_publicManager, &QNetworkAccessManager::finished,
            this, &ServerService::onPublicServersReply);

    QNetworkRequest request(url);
    m_publicManager->get(request);
}

void ServerService::onPublicServersReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    // Cache it
    m_storage.savePublicCache(data, QDateTime::currentDateTimeUtc());

    auto parsed = parsePublicXML(data);
    m_publicServers = parsed;
    emit publicServersUpdated(parsed);
}

QList<ServerInfo> ServerService::parsePublicXML(const QByteArray& data) const
{
    QList<ServerInfo> result;

    QDomDocument doc("servers");
    if (!doc.setContent(data))
        return result;

    QDomElement root(doc.documentElement());
    QDomElement element = root.firstChildElement();
    while (!element.isNull()) {
        ServerInfo info;
        if (parseServerElement(element, info)) {
            info.source = ServerSource::Public;
            info.lastSeen = QDateTime::currentDateTimeUtc();
            info.updateId();
            result.append(info);
        }
        element = element.nextSiblingElement();
    }
    return result;
}

bool ServerService::parseServerElement(const QDomElement& e, ServerInfo& out) const
{
    if (e.isNull())
        return false;

    // Reuse the same tags as .tt / server list XML
    QDomElement hostElem = e.firstChildElement("host");
    const QDomElement& h = hostElem.isNull() ? e : hostElem;

    out.name      = h.firstChildElement("name").text();
    out.host      = h.firstChildElement("ipaddr").text();
    out.tcpPort   = h.firstChildElement("tcpport").text().toInt();
    out.udpPort   = h.firstChildElement("udpport").text().toInt();
    out.encrypted = (h.firstChildElement("encrypted").text() == "true");

    out.username      = h.firstChildElement("username").text();
    out.password      = h.firstChildElement("password").text();
    out.nickname      = h.firstChildElement("nickname").text();
    out.statusMessage = h.firstChildElement("statusmsg").text();

    out.channel         = h.firstChildElement("channel").text();
    out.channelPassword = h.firstChildElement("chanpasswd").text();

    out.joinCode = h.firstChildElement("joincode").text();

    // Stats / listing info
    QDomElement stats = e.firstChildElement("stats");
    if (!stats.isNull()) {
        out.userCount = stats.firstChildElement("user-count").text().toInt();
        out.country   = stats.firstChildElement("country").text();
        out.motd      = stats.firstChildElement("motd").text();
        out.serverName = stats.firstChildElement("servername").text();
    }

    out.updateId();
    return out.isValid();
}

void ServerService::resolveJoinCode(const QString& code)
{
    if (!m_joinManager)
        m_joinManager = new QNetworkAccessManager(this);

    QString trimmed = code.trimmed();
    if (trimmed.isEmpty()) {
        emit joinCodeFailed(tr("Join code is empty"));
        return;
    }

    QUrl url(URL_SERVER_JOINCODE(QString::fromUtf8(QUrl::toPercentEncoding(trimmed))));
    connect(m_joinManager, &QNetworkAccessManager::finished,
            this, &ServerService::onJoinCodeReply);

    QNetworkRequest request(url);
    m_joinManager->get(request);
}

void ServerService::onJoinCodeReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit joinCodeFailed(tr("Failed to get server information."));
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QDomDocument doc("joincode");
    if (!doc.setContent(data)) {
        emit joinCodeFailed(tr("Invalid server response."));
        return;
    }

    QDomElement root(doc.documentElement());
    QDomElement element = root.firstChildElement();
    if (!element.isNull()) {
        ServerInfo info;
        if (parseServerElement(element, info)) {
            info.source = ServerSource::JoinCode;
            info.lastSeen = QDateTime::currentDateTimeUtc();
            info.updateId();
            emit joinCodeResolved(info);
            return;
        }
    }

    emit joinCodeFailed(tr("Join code incorrect"));
}

ServerInfo ServerService::parseDeepLink(const QString& urlStr) const
{
    ServerInfo info;
    QUrl url(urlStr);
    if (!url.isValid())
        return info;

    if (url.scheme().toLower() != QLatin1String("teamtalk"))
        return info;

    if (url.host().toLower() != QLatin1String("join"))
        return info;

    QUrlQuery q(url);
    info.host          = q.queryItemValue("host");
    info.tcpPort       = q.queryItemValue("tcp").toInt();
    info.udpPort       = q.queryItemValue("udp").toInt();
    info.username      = q.queryItemValue("user");
    info.password      = q.queryItemValue("pass");
    info.nickname      = q.queryItemValue("nick");
    info.statusMessage = q.queryItemValue("status");
    info.channel       = q.queryItemValue("chan");
    info.channelPassword = q.queryItemValue("chanpass");
    info.encrypted     = (q.queryItemValue("enc") == QLatin1String("1"));

    info.source  = ServerSource::DeepLink;
    info.lastSeen = QDateTime::currentDateTimeUtc();
    info.updateId();
    return info;
}

QString ServerService::toDeepLink(const ServerInfo& info) const
{
    QUrl url;
    url.setScheme(QStringLiteral("teamtalk"));
    url.setHost(QStringLiteral("join"));

    QUrlQuery q;
    q.addQueryItem("host", info.host);
    q.addQueryItem("tcp", QString::number(info.tcpPort));
    q.addQueryItem("udp", QString::number(info.udpPort));
    if (!info.username.isEmpty())
        q.addQueryItem("user", info.username);
    if (!info.password.isEmpty())
        q.addQueryItem("pass", info.password);
    if (!info.nickname.isEmpty())
        q.addQueryItem("nick", info.nickname);
    if (!info.statusMessage.isEmpty())
        q.addQueryItem("status", info.statusMessage);
    if (!info.channel.isEmpty())
        q.addQueryItem("chan", info.channel);
    if (!info.channelPassword.isEmpty())
        q.addQueryItem("chanpass", info.channelPassword);
    if (info.encrypted)
        q.addQueryItem("enc", "1");

    url.setQuery(q);
    return url.toString(QUrl::FullyEncoded);
}

ServerInfo ServerService::resolveQRPayload(const QString& payload)
{
    // Strategy:
    // 1) Try deep link
    // 2) Otherwise treat as join code (caller can call resolveJoinCode)
    ServerInfo info = parseDeepLink(payload);
    if (info.isValid())
        return info;

    // Not a deep link; we just return an empty ServerInfo here.
    // The UI can then call resolveJoinCode(payload).
    return ServerInfo{};
}

QList<ServerInfo> ServerService::importTTFile(const QString& path)
{
    QList<ServerInfo> result;

    QFile ttfile(QDir::fromNativeSeparators(path));
    if (!ttfile.open(QFile::ReadOnly))
        return result;

    QByteArray data = ttfile.readAll();
    QDomDocument doc;
    if (!doc.setContent(data))
        return result;

    QDomElement rootElement = doc.documentElement();
    if (rootElement.tagName() != TTFILE_ROOT)
        return result;

    QString version = rootElement.attribute("version");
    if (!versionSameOrLater(version, TTFILE_VERSION))
        return result;

    QDomElement hostElement = rootElement.firstChildElement("host");
    while (!hostElement.isNull()) {
        ServerInfo info;
        if (parseServerElement(hostElement, info)) {
            info.source = ServerSource::ImportedTT;
            info.lastSeen = QDateTime::currentDateTimeUtc();
            info.updateId();
            result.append(info);
        }
        hostElement = hostElement.nextSiblingElement("host");
    }

    return result;
}

bool ServerService::exportTTFileSingle(const QString& path, const QList<ServerInfo>& servers)
{
    if (servers.isEmpty())
        return false;

    QDomDocument doc;
    QDomProcessingInstruction xmlDecl = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDecl);

    QDomElement root = doc.createElement(TTFILE_ROOT);
    root.setAttribute("version", TTFILE_VERSION);
    doc.appendChild(root);

    for (const auto& info : servers) {
        HostEntry entry;
        entry.name      = info.name;
        entry.ipaddr    = info.host;
        entry.tcpport   = info.tcpPort;
        entry.udpport   = info.udpPort;
        entry.encrypted = info.encrypted;
        entry.username  = info.username;
        entry.password  = info.password;
        entry.nickname  = info.nickname;
        entry.statusmsg = info.statusMessage;
        entry.channel   = info.channel;
        entry.chanpasswd = info.channelPassword;

        QByteArray xml = generateTTFile(entry);
        QDomDocument entryDoc;
        entryDoc.setContent(xml);
        QDomElement hostElement = entryDoc.documentElement().firstChildElement("host");
        root.appendChild(doc.importNode(hostElement, true));
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(doc.toByteArray());
    file.close();
    return true;
}

bool ServerService::exportTTFileMultiple(const QString& dir, const QList<ServerInfo>& servers)
{
    if (servers.isEmpty())
        return false;

    QDir d(dir);
    if (!d.exists())
        return false;

    for (const auto& info : servers) {
        HostEntry entry;
        entry.name      = info.name;
        entry.ipaddr    = info.host;
        entry.tcpport   = info.tcpPort;
        entry.udpport   = info.udpPort;
        entry.encrypted = info.encrypted;
        entry.username  = info.username;
        entry.password  = info.password;
        entry.nickname  = info.nickname;
        entry.statusmsg = info.statusMessage;
        entry.channel   = info.channel;
        entry.chanpasswd = info.channelPassword;

        QByteArray xml = generateTTFile(entry);
        QString safeName = info.name;
        if (safeName.isEmpty())
            safeName = QStringLiteral("Server");
        safeName.replace(QRegExp("[^A-Za-z0-9_-]"), "_");

        QString filePath = d.filePath(safeName + QStringLiteral(".tt"));
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
            continue;
        file.write(xml);
        file.close();
    }
    return true;
}

QList<ServerInfo> ServerService::mergeServerLists(const QList<ServerInfo>& a,
                                                  const QList<ServerInfo>& b) const
{
    QMap<QString, ServerInfo> map;
    for (const auto& s : a)
        map.insert(s.id, s);
    for (const auto& s : b)
        map.insert(s.id, s); // b overwrites a on conflict
    return map.values();
}

QList<ServerInfo> ServerService::loadPersonalServers() const
{
    return m_storage.loadPersonalServers();
}

void ServerService::savePersonalServers(const QList<ServerInfo>& servers)
{
    m_storage.savePersonalServers(servers);
}

QList<ServerInfo> ServerService::loadLatestHosts() const
{
    return m_storage.loadLatestHosts();
}

void ServerService::saveLatestHosts(const QList<ServerInfo>& hosts)
{
    m_storage.saveLatestHosts(hosts);
}
