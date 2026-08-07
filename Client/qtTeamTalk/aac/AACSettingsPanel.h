#pragma once

#include <QWidget>
#include <QMap>
#include <QVector>

class AACToggle;
class AACKeyButton;
class AACAccessibilityManager;

class AACSettingsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit AACSettingsPanel(AACAccessibilityManager* mgr,
                              QWidget* parent = nullptr);

    void addSection(const QString& title);
    void addToggle(const QString& key, AACToggle* toggle);

    void loadFrom(const QMap<QString, bool>& current);
    QMap<QString, bool> workingCopy() const;

signals:
    void applied(const QMap<QString, bool>& newValues);
    void cancelled();

private slots:
    void onApply();
    void onCancel();

private:
    AACAccessibilityManager* m_mgr = nullptr;

    QMap<QString, AACToggle*> m_toggles;
    QMap<QString, bool>       m_working;

    QVector<QWidget*> m_sections;

    AACKeyButton* m_btnApply = nullptr;
    AACKeyButton* m_btnCancel = nullptr;

    void buildFooter();
    void speakPanelEntry();
};
