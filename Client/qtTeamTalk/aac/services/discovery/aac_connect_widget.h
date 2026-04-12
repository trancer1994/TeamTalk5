#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "serverinfo.h"

class AACConnectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AACConnectWidget(QWidget* parent = nullptr);

public slots:
    void loadServerInfo(const ServerInfo& info);

signals:
    void connectRequested(const ServerInfo& info);
    void cancelled();

private slots:
    void onConnectPressed();
    void onBackPressed();

private:
    void applyDirectFill(const ServerInfo& info);
    void clearFocusSafely();

private:
    QLineEdit* m_hostEdit = nullptr;
    QSpinBox*  m_portEdit = nullptr;
    QLineEdit* m_userEdit = nullptr;
    QLineEdit* m_passEdit = nullptr;
    QLineEdit* m_nickEdit = nullptr;
    QLineEdit* m_channelEdit = nullptr;
    QLineEdit* m_channelPassEdit = nullptr;

    QPushButton* m_connectButton = nullptr;
    QPushButton* m_backButton = nullptr;

    bool m_ready = false;
};
