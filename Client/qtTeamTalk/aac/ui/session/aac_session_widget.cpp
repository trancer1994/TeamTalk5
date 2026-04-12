#include "AACSessionWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

AACSessionWidget::AACSessionWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void AACSessionWidget::setupUi()
{
    m_userList = new QListWidget(this);
    m_userList->setSelectionMode(QAbstractItemView::NoSelection);
    m_userList->setFocusPolicy(Qt::NoFocus);

    m_transmitButton = new QPushButton(tr("Transmit"), this);
    m_leaveButton = new QPushButton(tr("Leave Channel"), this);
    m_settingsButton = new QPushButton(tr("Settings"), this);
    m_aacButton = new QPushButton(tr("AAC Access"), this);
    m_nicknameButton = new QPushButton(tr("Change Nickname"), this);
    m_joinButton = new QPushButton(tr("Join Channel"), this);
    m_statusLabel = new QLabel(tr("Not transmitting"), this);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_userList);

    layout->addWidget(m_transmitButton);
    layout->addWidget(m_joinButton);
    layout->addWidget(m_nicknameButton);
    layout->addWidget(m_settingsButton);
    layout->addWidget(m_aacButton);
    layout->addWidget(m_leaveButton);

    connect(m_transmitButton, &QPushButton::clicked,
            this, &AACSessionWidget::onTransmitClicked);
    connect(m_leaveButton, &QPushButton::clicked,
            this, &AACSessionWidget::onLeaveClicked);
    connect(m_settingsButton, &QPushButton::clicked,
            this, &AACSessionWidget::onSettingsClicked);
    connect(m_aacButton, &QPushButton::clicked,
            this, &AACSessionWidget::onAACClicked);
    connect(m_nicknameButton, &QPushButton::clicked,
            this, &AACSessionWidget::onNicknameClicked);
    connect(m_joinButton, &QPushButton::clicked,
            this, &AACSessionWidget::onJoinChannelClicked);
}

void AACSessionWidget::setUsers(const QStringList& users)
{
    m_userList->clear();
    m_userList->addItems(users);
}

void AACSessionWidget::setTransmitActive(bool active)
{
    m_transmitting = active;
    m_transmitButton->setText(active ? tr("Stop Transmitting")
                                     : tr("Transmit"));
    m_statusLabel->setText(active ? tr("Transmitting")
                                  : tr("Not transmitting"));
}

void AACSessionWidget::onTransmitClicked()
{
    setTransmitActive(!m_transmitting);
    emit transmitToggled(m_transmitting);
}

void AACSessionWidget::onLeaveClicked()
{
    emit leaveRequested();
}

void AACSessionWidget::onSettingsClicked()
{
    emit settingsRequested();
}

void AACSessionWidget::onAACClicked()
{
    emit aacSettingsRequested();
}

void AACSessionWidget::onNicknameClicked()
{
    emit changeNicknameRequested();
}

void AACSessionWidget::onJoinChannelClicked()
{
    emit joinChannelRequested();
}
