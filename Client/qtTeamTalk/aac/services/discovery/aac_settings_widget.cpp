#include "aac_settings_widget.h"

#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

AACSettingsWidget::AACSettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void AACSettingsWidget::setupUi()
{
    m_transmitModeCombo = new QComboBox(this);
    m_transmitModeCombo->addItem(tr("Tap to toggle"));
    m_transmitModeCombo->addItem(tr("Hold to talk"));
    m_transmitModeCombo->addItem(tr("VOX"));
    m_transmitModeCombo->addItem(tr("Noise gate"));
    m_transmitModeCombo->addItem(tr("Timed"));

    m_backButton = new QPushButton(tr("Back"), this);
    m_statusLabel = new QLabel(tr("Settings"), this);

    auto* form = new QFormLayout;
    form->addRow(tr("Transmit mode"), m_transmitModeCombo);

    auto* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(m_backButton);

    auto* layout = new QVBoxLayout;
    layout->addWidget(m_statusLabel);
    layout->addLayout(form);
    layout->addLayout(buttons);

    setLayout(layout);

    connect(m_backButton, &QPushButton::clicked,
            this, &AACSettingsWidget::onBackClicked);
}

void AACSettingsWidget::onBackClicked()
{
    emit backRequested();
}
