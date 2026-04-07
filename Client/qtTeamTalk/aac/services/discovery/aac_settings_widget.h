#pragma once

#include <QWidget>

class QComboBox;
class QPushButton;
class QLabel;

class AACSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AACSettingsWidget(QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void onBackClicked();

private:
    void setupUi();

private:
    QComboBox* m_transmitModeCombo = nullptr;
    QPushButton* m_backButton = nullptr;
    QLabel* m_statusLabel = nullptr;
};
