#pragma once

#include <QObject>
#include <QJsonObject>

class AACUI;

namespace AAC {

class AACGridModel;
class AACMessageBar;

class AACModule : public QObject
{
    Q_OBJECT

public:
    AACModule();

private:
    void loadCoreVocabulary();
    void connectSignals();

private slots:
    void onSymbolActivated(const QString& label);
    void onMessageReady(const QString& text);

    AACUI* m_ui;
    AACGridModel* m_model;
    AACMessageBar* m_messageBar;
    QJsonObject m_coreJson;
};

} // namespace AAC
