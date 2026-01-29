#pragma once

#include <QObject>
#include <QJsonObject>

class AACUI;

namespace AAC {

class AACModule : public QObject
{
    Q_OBJECT

public:
    AACModule();

private:
    void loadCoreVocabulary();

    AACUI* m_ui;
    QJsonObject m_coreJson;
};

} // namespace AAC
