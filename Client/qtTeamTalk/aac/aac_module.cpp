#include "aac_module.h"
#include "aacui.h"
#include "aac_grid_model.h"
#include "aac_message_bar.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>

namespace AAC {

AACModule::AACModule()
    : m_ui(new AACUI())
    , m_model(new AACGridModel())
    , m_messageBar(new AACMessageBar())
{
    loadCoreVocabulary();
    m_model->loadFromJson(m_coreJson);

    m_ui->setModel(m_model);
    connectSignals();
}

void AACModule::connectSignals()
{
    QObject::connect(m_ui, &AACUI::symbolActivated,
                     this, &AACModule::onSymbolActivated);

    QObject::connect(m_ui, &AACUI::categorySelected,
                     this, &AACModule::onCategorySelected);

    QObject::connect(m_messageBar, &AACMessageBar::messageReady,
                     this, &AACModule::onMessageReady);
}

void AACModule::onCategorySelected(const QString& categoryId)
{
    qDebug() << "AACModule: category selected:" << categoryId;
    // later: load different JSON section into m_model and call m_ui->setModel(m_model) again
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
void AACModule::onSymbolActivated(const QString& label)
{
    m_messageBar->appendSymbol(label);
}

void AACModule::onMessageReady(const QString& text)
{
    qDebug() << "AACModule: message ready:" << text;
    // later: hook into TTS / sending
}

} // namespace AAC
