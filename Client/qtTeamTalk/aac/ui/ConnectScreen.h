#pragma once
#include "AACScreenBase.h"

struct ServerInfo {
    QString host;
    quint16 port;
    QString username;
    QString password;
};

enum class ConnectMode {
    MetadataDriven,
    Manual
};

class QLineEdit;
class AACKeyButton;

class ConnectScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit ConnectScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setMode(ConnectMode mode);
    void setMetadata(const QString& host, quint16 port);
void showReconnectSpinner();
void hideReconnectSpinner();
    void setUiEnabled(bool enabled);

protected:
    void keyPressEvent(QKeyEvent* e) override;

signals:
    void cycleNextMode();
    void cyclePrevMode();
    void connectRequested(const ServerInfo& info);
    void backRequested();
void reconnectRequested();
void reconnectCancelled();

private:
    void speak(const QString& text);
    ConnectMode m_mode = ConnectMode::Manual;

    QLineEdit* m_hostEdit = nullptr;
    QLineEdit* m_portEdit = nullptr;
    QLineEdit* m_userEdit = nullptr;
    QLineEdit* m_passEdit = nullptr;
QWidget* m_reconnectOverlay = nullptr;
QLabel*  m_reconnectLabel   = nullptr;

    AACKeyButton* m_connectBtn = nullptr;
    AACKeyButton* m_backBtn = nullptr;
    AACKeyButton* m_reconnectBtn = nullptr; // NEW
};
