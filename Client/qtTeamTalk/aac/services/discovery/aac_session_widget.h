#pragma once

#include <QWidget>

class QListWidget;
class QPushButton;
class QLabel;

class AACSessionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AACSessionWidget(QWidget* parent = nullptr);

    void setUsers(const QStringList& users);
    void setTransmitActive(bool active);

signals:
    void transmitToggled(bool active);
    void leaveRequested();
    void settingsRequested();

private slots:
    void onTransmitClicked();
    void onLeaveClicked();
    void onSettingsClicked();

private:
    void setupUi();

private:
    QListWidget* m_userList = nullptr;
    QPushButton* m_transmitButton = nullptr;
    QPushButton* m_leaveButton = nullptr;
    QPushButton* m_settingsButton = nullptr;
    QLabel* m_statusLabel = nullptr;

    bool m_transmitting = false;
};
