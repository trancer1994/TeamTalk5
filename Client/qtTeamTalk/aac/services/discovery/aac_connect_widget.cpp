#include "aac_connect_widget.h"

AACConnectWidget::AACConnectWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    m_hostEdit = new QLineEdit(this);
    m_portEdit = new QSpinBox(this);
    m_portEdit->setRange(1, 65535);

    m_userEdit = new QLineEdit(this);
    m_passEdit = new QLineEdit(this);
    m_passEdit->setEchoMode(QLineEdit::Password);

    m_nickEdit = new QLineEdit(this);
    m_channelEdit = new QLineEdit(this);
    m_channelPassEdit = new QLineEdit(this);
    m_channelPassEdit->setEchoMode(QLineEdit::Password);

    m_connectButton = new QPushButton("Connect", this);
    m_backButton = new QPushButton("Back", this);

    layout->addWidget(m_hostEdit);
    layout->addWidget(m_portEdit);
    layout->addWidget(m_userEdit);
    layout->addWidget(m_passEdit);
    layout->addWidget(m_nickEdit);
    layout->addWidget(m_channelEdit);
    layout->addWidget(m_channelPassEdit);
    layout->addWidget(m_connectButton);
    layout->addWidget(m_backButton);

    connect(m_connectButton, &QPushButton::clicked,
            this, &AACConnectWidget::onConnectPressed);

    connect(m_backButton, &QPushButton::clicked,
            this, &AACConnectWidget::onBackPressed);
}

void AACConnectWidget::loadServerInfo(const ServerInfo& info)
{
    applyDirectFill(info);
    clearFocusSafely();
    m_ready = true;
}

void AACConnectWidget::applyDirectFill(const ServerInfo& info)
{
    m_hostEdit->setText(info.host);
    m_portEdit->setValue(info.port);

    if (!info.username.isEmpty())
        m_userEdit->setText(info.username);

    if (!info.password.isEmpty())
        m_passEdit->setText(info.password);

    if (!info.nickname.isEmpty())
        m_nickEdit->setText(info.nickname);

    if (!info.channel.isEmpty())
        m_channelEdit->setText(info.channel);

    if (!info.channelPassword.isEmpty())
        m_channelPassEdit->setText(info.channelPassword);
}

void AACConnectWidget::clearFocusSafely()
{
    setFocus(Qt::NoFocusReason);
    m_hostEdit->clearFocus();
    m_portEdit->clearFocus();
    m_userEdit->clearFocus();
    m_passEdit->clearFocus();
    m_nickEdit->clearFocus();
    m_channelEdit->clearFocus();
    m_channelPassEdit->clearFocus();
}

void AACConnectWidget::onConnectPressed()
{
    if (!m_ready)
        return;

    ServerInfo info;
    info.host = m_hostEdit->text();
    info.port = m_portEdit->value();
    info.username = m_userEdit->text();
    info.password = m_passEdit->text();
    info.nickname = m_nickEdit->text();
    info.channel = m_channelEdit->text();
    info.channelPassword = m_channelPassEdit->text();

    emit connectRequested(info);
}

void AACConnectWidget::onBackPressed()
{
    emit cancelled();
}
