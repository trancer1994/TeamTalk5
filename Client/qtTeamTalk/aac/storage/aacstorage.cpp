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

    if (root.contains("dwellConfig")) {
        AACDwellConfig c;
        const QJsonObject o = root["dwellConfig"].toObject();
        c.dwellDurationMs = o["dwellDurationMs"].toInt();
        mgr.setDwellConfig(c);
    }

    if (root.contains("scanningConfig")) {
        AACScanningConfig c;
        const QJsonObject o = root["scanningConfig"].toObject();
        c.stepIntervalMs = o["stepIntervalMs"].toInt();
        mgr.setScanningConfig(c);
    }

    if (root.contains("layoutConfig")) {
        AACLayoutConfig c;
        const QJsonObject o = root["layoutConfig"].toObject();
        c.oneHandRightSide = o["oneHandRightSide"].toBool();
        mgr.setLayoutConfig(c);
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
}

void AACStorage::persist(const AACAccessibilityManager& mgr)
{
    QJsonObject root;

    root["activeCategory"]    = mgr.activeCategory();
    root["profile"]           = static_cast<int>(mgr.profile());
    root["predictionEnabled"] = mgr.predictionEnabled();

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
        const AACDwellConfig c = mgr.dwellConfig();
        QJsonObject o;
        o["dwellDurationMs"] = c.dwellDurationMs;
        root["dwellConfig"] = o;
    }

    {
        const AACScanningConfig c = mgr.scanningConfig();
        QJsonObject o;
        o["stepIntervalMs"] = c.stepIntervalMs;
        root["scanningConfig"] = o;
    }

    {
        const AACLayoutConfig c = mgr.layoutConfig();
        QJsonObject o;
        o["oneHandRightSide"] = c.oneHandRightSide;
        root["layoutConfig"] = o;
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
