#include "aac_connect_widget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "Client/qtTeamTalk/aac/models/serverinfo.h"

AACConnectWidget::AACConnectWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void AACConnectWidget::setupUi()
{
    m_hostEdit = new QLineEdit(this);
    m_tcpEdit = new QLineEdit(this);
    m_udpEdit = new QLineEdit(this);
    m_userEdit = new QLineEdit(this);
    m_passEdit = new QLineEdit(this);
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_nickEdit = new QLineEdit(this);
    m_statusEdit = new QLineEdit(this);
    m_channelEdit = new QLineEdit(this);
    m_chanPassEdit = new QLineEdit(this);
    m_chanPassEdit->setEchoMode(QLineEdit::Password);

    m_connectButton = new QPushButton(tr("Connect"), this);
    m_backButton = new QPushButton(tr("Back"), this);
    m_statusLabel = new QLabel(tr("Not connected"), this);

    auto* form = new QFormLayout;
    form->addRow(tr("Host"), m_hostEdit);
    form->addRow(tr("TCP port"), m_tcpEdit);
    form->addRow(tr("UDP port"), m_udpEdit);
    form->addRow(tr("Username"), m_userEdit);
    form->addRow(tr("Password"), m_passEdit);
    form->addRow(tr("Nickname"), m_nickEdit);
    form->addRow(tr("Status"), m_statusEdit);
    form->addRow(tr("Channel"), m_channelEdit);
    form->addRow(tr("Channel password"), m_chanPassEdit);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(m_connectButton);
    buttons->addStretch();
    buttons->addWidget(m_backButton);

    auto* layout = new QVBoxLayout;
    layout->addWidget(m_statusLabel);
    layout->addLayout(form);
    layout->addLayout(buttons);

    setLayout(layout);

    connect(m_connectButton, &QPushButton::clicked,
            this, &AACConnectWidget::onConnectClicked);
    connect(m_backButton, &QPushButton::clicked,
            this, &AACConnectWidget::backRequested);
}

void AACConnectWidget::setServerInfo(const ServerInfo& info)
{
    m_hostEdit->setText(info.host);
    m_tcpEdit->setText(QString::number(info.tcpPort));
    m_udpEdit->setText(QString::number(info.udpPort));
    m_userEdit->setText(info.username);
    m_passEdit->setText(info.password);
    m_nickEdit->setText(info.nickname);
    m_statusEdit->setText(info.statusMessage);
    m_channelEdit->setText(info.channel);
    m_chanPassEdit->setText(info.channelPassword);
}

ServerInfo AACConnectWidget::collectInfo() const
{
    ServerInfo info;
    info.host = m_hostEdit->text();
    info.tcpPort = m_tcpEdit->text().toInt();
    info.udpPort = m_udpEdit->text().toInt();
    info.username = m_userEdit->text();
    info.password = m_passEdit->text();
    info.nickname = m_nickEdit->text();
    info.statusMessage = m_statusEdit->text();
    info.channel = m_channelEdit->text();
    info.channelPassword = m_chanPassEdit->text();
    return info;
}

void AACConnectWidget::onConnectClicked()
{
    ServerInfo info = collectInfo();
    emit connectRequested(info);
}
