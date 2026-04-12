#pragma once

#include <QWidget>
#include <QStringList>

class QListWidget;
class QPushButton;
class QLabel;

class AACSessionWidget : public QWidget {
    Q_OBJECT
public:
    explicit AACSessionWidget(QWidget* parent = nullptr);

    void setUsers(const QStringList& users);
    void setTransmitActive(bool active);

signals:
    void transmitToggled(bool active);
    void leaveRequested();
    void settingsRequested();
    void aacSettingsRequested();
    void changeNicknameRequested();
    void joinChannelRequested();

private slots:
    void onTransmitClicked();
    void onLeaveClicked();
    void onSettingsClicked();
    void onAACClicked();
    void onNicknameClicked();
    void onJoinChannelClicked();

private:
    void setupUi();

    QListWidget* m_userList = nullptr;
    QPushButton* m_transmitButton = nullptr;
    QPushButton* m_leaveButton = nullptr;
    QPushButton* m_settingsButton = nullptr;
    QPushButton* m_aacButton = nullptr;
    QPushButton* m_nicknameButton = nullptr;
    QPushButton* m_joinButton = nullptr;
    QLabel* m_statusLabel = nullptr;

    bool m_transmitting = false;
};
