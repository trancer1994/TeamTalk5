#pragma once

#include <QWidget>
#include <QPointer>

class QLineEdit;
class QPushButton;
class QLabel;

struct ServerInfo;

class AACConnectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AACConnectWidget(QWidget* parent = nullptr);

    void setServerInfo(const ServerInfo& info);

signals:
    void connectRequested(const ServerInfo& info);
    void backRequested();

private slots:
    void onConnectClicked();

private:
    void setupUi();
    ServerInfo collectInfo() const;

private:
    QLineEdit* m_hostEdit = nullptr;
    QLineEdit* m_tcpEdit = nullptr;
    QLineEdit* m_udpEdit = nullptr;
    QLineEdit* m_userEdit = nullptr;
    QLineEdit* m_passEdit = nullptr;
    QLineEdit* m_nickEdit = nullptr;
    QLineEdit* m_statusEdit = nullptr;
    QLineEdit* m_channelEdit = nullptr;
    QLineEdit* m_chanPassEdit = nullptr;

    QPushButton* m_connectButton = nullptr;
    QPushButton* m_backButton = nullptr;
    QLabel* m_statusLabel = nullptr;
};
