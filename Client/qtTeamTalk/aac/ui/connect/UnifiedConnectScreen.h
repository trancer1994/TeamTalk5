#pragma once

#include "AACScreenBase.h"

#include <QWidget>
#include <QVector>
#include <QDateTime>

class QLabel;
class QLineEdit;
class QPushButton;
class QListWidget;

struct UnifiedServerEntry {
    QString name;
    QString host;
    QString port;
    bool accountBased = false;
};

class UnifiedConnectScreen : public AACScreenBase
{
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

    void setServerList(const QVector<UnifiedServerEntry>& servers);

public slots:
    void onScanningChanged(bool scanning);
    void onLastUpdatedChanged(const QDateTime& dt);

    void setErrorMessage(const QString& message);

signals:
    void connectRequested(const QString& host,
                          const QString& port,
                          const QString& username,
                          const QString& password);

    void manualConnectRequested();
    void backRequested();

    void publicServersRequested();
    void lanServersRequested();
    void recentServersRequested();
    void joinCodeEntered(const QString& code);
    void qrScanRequested();

    void scanAgainRequested();

private:
    void buildUi();

    QString makeKey(const QString& host, const QString& port) const;
    void updateEmptyLabel();

    Mode m_mode = Mode::MetadataDriven;

    QPushButton* m_publicButton = nullptr;
    QPushButton* m_lanButton = nullptr;
    QPushButton* m_recentButton = nullptr;
    QPushButton* m_joinCodeButton = nullptr;
    QPushButton* m_qrButton = nullptr;
    QListWidget* m_serverList = nullptr;
    QLineEdit*   m_joinCodeEdit = nullptr;

    QLabel* m_scanningLabel = nullptr;
    QPushButton* m_scanAgainButton = nullptr;

    QLabel* m_errorLabel = nullptr;
    QLabel* m_emptyLabel = nullptr;

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

    QVector<UnifiedServerEntry> m_servers;

    QString m_lastSelectedKey;
};
