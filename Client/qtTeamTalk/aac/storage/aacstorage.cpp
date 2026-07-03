#include "aacstorage.h"

#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "AACFramework.h"

AACStorage::AACStorage() = default;

QString AACStorage::storagePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/aac_state.json";
}

QJsonObject AACStorage::loadJson() const
{
    QFile f(storagePath());
    if (!f.exists())
        return QJsonObject();

    if (!f.open(QIODevice::ReadOnly))
        return QJsonObject();

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject())
        return QJsonObject();

    return doc.object();
}

void AACStorage::saveJson(const QJsonObject& obj) const
{
    QFile f(storagePath());
    if (!f.open(QIODevice::WriteOnly))
        return;

    f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

int AACStorage::loadUserVolume(const QString& userId) const
{
    auto it = m_userVolumes.constFind(userId);
    if (it == m_userVolumes.constEnd())
        return -1;
    return it.value();
}

void AACStorage::saveUserVolume(const QString& userId, int volume)
{
    m_userVolumes[userId] = volume;
}
void AACStorage::hydrate(AACAccessibilityManager& mgr)
{
    const QJsonObject root = loadJson();
    if (root.isEmpty())
        return;

    if (root.contains("activeCategory"))
        mgr.setActiveCategory(root["activeCategory"].toString());

    if (root.contains("profile"))
        mgr.setProfile(static_cast<AACProfile>(root["profile"].toInt()));

    if (root.contains("predictionEnabled"))
        mgr.setPredictionEnabled(root["predictionEnabled"].toBool());

    if (root.contains("modes")) {
        AACModeFlags m;
        const QJsonObject o = root["modes"].toObject();
        m.largeTargets    = o["largeTargets"].toBool();
        m.dwell           = o["dwell"].toBool();
        m.scanning        = o["scanning"].toBool();
        m.auditoryFeedback= o["auditoryFeedback"].toBool();
        m.hapticFeedback  = o["hapticFeedback"].toBool();
        m.deepWells       = o["deepWells"].toBool();
        m.oneHandLayout   = o["oneHandLayout"].toBool();
        m.ultraMinimal    = o["ultraMinimal"].toBool();
        m.predictiveStrip = o["predictiveStrip"].toBool();
        mgr.setModes(m);
    }

    if (root.contains("speechConfig")) {
        AACSpeechConfig c;
        const QJsonObject o = root["speechConfig"].toObject();
        c.voiceName          = o["voiceName"].toString();
        c.rate               = o["rate"].toDouble();
        c.pitch              = o["pitch"].toDouble();
        c.volume             = o["volume"].toDouble();
        c.speakAsYouTypeMode = static_cast<AACSpeechConfig::SpeakAsYouTypeMode>(o["speakAsYouTypeMode"].toInt());
        c.echoOnSend         = o["echoOnSend"].toBool();
        c.highIntelligible   = o["highIntelligible"].toBool();
        c.lowIntensity       = o["lowIntensity"].toBool();
        mgr.setSpeechConfig(c);
    }

if (root.contains("userVolumes")) {
    const QJsonObject o = root["userVolumes"].toObject();
    m_userVolumes.clear();
    for (auto it = o.begin(); it != o.end(); ++it) {
        m_userVolumes.insert(it.key(), it.value().toInt());
    }

    // Push into AACAccessibilityManager
    for (auto it = m_userVolumes.begin(); it != m_userVolumes.end(); ++it) {
        mgr.setUserVolume(it.key(), it.value());
    }
}
}

void AACStorage::persist(const AACAccessibilityManager& mgr)
{
    QJsonObject root;

    root["activeCategory"]    = mgr.activeCategory();
    root["profile"]           = static_cast<int>(mgr.profile());
    root["predictionEnabled"] = mgr.predictionEnabled();

{
    QJsonObject o;
    const QMap<QString,int> vols = mgr.allUserVolumes();
    for (auto it = vols.begin(); it != vols.end(); ++it) {
        o[it.key()] = it.value();
    }
    root["userVolumes"] = o;
}
    {
        const AACModeFlags m = mgr.modes();
        QJsonObject o;
        o["largeTargets"]     = m.largeTargets;
        o["dwell"]            = m.dwell;
        o["scanning"]         = m.scanning;
        o["auditoryFeedback"] = m.auditoryFeedback;
        o["hapticFeedback"]   = m.hapticFeedback;
        o["deepWells"]        = m.deepWells;
        o["oneHandLayout"]    = m.oneHandLayout;
        o["ultraMinimal"]     = m.ultraMinimal;
        o["predictiveStrip"]  = m.predictiveStrip;
        root["modes"] = o;
    }

    {
        const AACSpeechConfig c = mgr.speechConfig();
        QJsonObject o;
        o["voiceName"]          = c.voiceName;
        o["rate"]               = c.rate;
        o["pitch"]              = c.pitch;
        o["volume"]             = c.volume;
        o["speakAsYouTypeMode"] = static_cast<int>(c.speakAsYouTypeMode);
        o["echoOnSend"]         = c.echoOnSend;
        o["highIntelligible"]   = c.highIntelligible;
        o["lowIntensity"]       = c.lowIntensity;
        root["speechConfig"] = o;
    }

    saveJson(root);
}
