#include "aac_module.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace AAC {

AACModule::AACModule()
    : m_ui(new AACUI())
{
    loadCoreVocabulary();
}

void AACModule::loadCoreVocabulary()
{
    QFile file(":/aac/core48.json");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "AACModule: Failed to open embedded core48.json";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "AACModule: JSON parse error in core48.json:" << err.errorString();
        return;
    }

    m_coreJson = doc.object();

    // Later: pass this into your grid model or UI
    qDebug() << "AACModule: Loaded core vocabulary with keys:" << m_coreJson.keys();
}

} // namespace AAC
