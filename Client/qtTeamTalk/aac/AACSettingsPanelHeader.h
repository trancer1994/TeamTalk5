#pragma once

#include <QWidget>
#include <QString>

class AACKeyButton;
class AACAccessibilityManager;

class AACSettingsPanelHeader : public QWidget
{
    Q_OBJECT
public:
    explicit AACSettingsPanelHeader(AACAccessibilityManager* mgr,
                                    const QString& title,
                                    const QString& helpText = QString(),
                                    QWidget* parent = nullptr);

signals:
    void backRequested();

private:
    AACAccessibilityManager* m_mgr = nullptr;

    QString m_title;
    QString m_helpText;

    AACKeyButton* m_btnBack = nullptr;

    void speakEntry();
    void applyVisuals();
};
