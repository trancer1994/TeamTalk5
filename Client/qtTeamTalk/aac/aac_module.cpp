#include "aac_module.h"
#include "aacui.h"
#include "aac_grid_model.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>

namespace AAC {

AACModule::AACModule()
    : m_ui(new AACUI())
    , m_model(new AACGridModel())
{
    loadCoreVocabulary();
    m_model->loadFromJson(m_coreJson);

    m_ui->setModel(m_model);   // we add this next
}

void AACModule::loadCoreVocabulary()
{
    QFile file(":/aac/core48.json");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "AACModule: Failed to open embedded core48.json";
        return;
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "AACModule: JSON parse error:" << err.errorString();
        return;
    }

    m_coreJson = doc.object();
}

} // namespace AAC
