#include "aac_session_widget.h"

#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
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

    m_transmitButton = new QPushButton(tr("Transmit"), this);
    m_leaveButton = new QPushButton(tr("Leave channel"), this);
    m_settingsButton = new QPushButton(tr("Settings"), this);
    m_statusLabel = new QLabel(tr("Not transmitting"), this);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(m_transmitButton);
    buttons->addWidget(m_settingsButton);
    buttons->addStretch();
    buttons->addWidget(m_leaveButton);

    auto* layout = new QVBoxLayout;
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_userList);
    layout->addLayout(buttons);

    setLayout(layout);

    connect(m_transmitButton, &QPushButton::clicked,
            this, &AACSessionWidget::onTransmitClicked);
    connect(m_leaveButton, &QPushButton::clicked,
            this, &AACSessionWidget::onLeaveClicked);
    connect(m_settingsButton, &QPushButton::clicked,
            this, &AACSessionWidget::onSettingsClicked);
}

void AACSessionWidget::setUsers(const QStringList& users)
{
    m_userList->clear();
    m_userList->addItems(users);
}

void AACSessionWidget::setTransmitActive(bool active)
{
    m_transmitting = active;
    m_transmitButton->setText(active ? tr("Stop transmitting")
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
