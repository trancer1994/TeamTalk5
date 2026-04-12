#pragma once

#include "AACScreen.h"
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

class UnifiedConnectScreen : public AACScreen {
    Q_OBJECT
public:
    enum class Mode {
        MetadataDriven,
        Manual
    };

    explicit UnifiedConnectScreen(AACAccessibilityManager* aac,
                                  QWidget* parent = nullptr);

    void configureForMetadata(const QString& serverName,
                              const QString& host,
                              const QString& port,
                              bool accountBased);

    void configureForManual();

signals:
    void connectRequested(const QString& host,
                          const QString& port,
                          const QString& username,
                          const QString& password);

    void manualConnectRequested();
    void backRequested();

private:
    void buildUi();

    Mode m_mode = Mode::MetadataDriven;

    QLabel* m_serverNameLabel = nullptr;
    QLabel* m_hostLabel = nullptr;
    QLabel* m_portLabel = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;

    QLineEdit* m_manualHostEdit = nullptr;
    QLineEdit* m_manualPortEdit = nullptr;

    QPushButton* m_connectButton = nullptr;
    QPushButton* m_manualButton = nullptr;
    QPushButton* m_backButton = nullptr;
};
