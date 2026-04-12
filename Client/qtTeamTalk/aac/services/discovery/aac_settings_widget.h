#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

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
