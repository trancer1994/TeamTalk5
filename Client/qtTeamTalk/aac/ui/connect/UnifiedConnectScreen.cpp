#include "UnifiedConnectScreen.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

UnifiedConnectScreen::UnifiedConnectScreen(AACAccessibilityManager* aac,
                                           QWidget* parent)
    : AACScreen(aac, parent)
{
    buildUi();
}

void UnifiedConnectScreen::buildUi()
{
    auto* layout = new QVBoxLayout(this);

    m_serverNameLabel = new QLabel(this);
    layout->addWidget(m_serverNameLabel);
    registerInteractive(m_serverNameLabel);

    m_hostLabel = new QLabel(this);
    layout->addWidget(m_hostLabel);
    registerInteractive(m_hostLabel);

    m_portLabel = new QLabel(this);
    layout->addWidget(m_portLabel);
    registerInteractive(m_portLabel);

    m_usernameEdit = new QLineEdit(this);
    layout->addWidget(m_usernameEdit);
    registerInteractive(m_usernameEdit);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passwordEdit);
    registerInteractive(m_passwordEdit);

    m_manualHostEdit = new QLineEdit(this);
    m_manualPortEdit = new QLineEdit(this);
    layout->addWidget(m_manualHostEdit);
    layout->addWidget(m_manualPortEdit);
    registerInteractive(m_manualHostEdit);
    registerInteractive(m_manualPortEdit);

    m_connectButton = new QPushButton(tr("Connect"), this);
    layout->addWidget(m_connectButton);
    registerInteractive(m_connectButton, true);

    m_manualButton = new QPushButton(tr("Manual Connect"), this);
    layout->addWidget(m_manualButton);
    registerInteractive(m_manualButton);

    m_backButton = new QPushButton(tr("Back"), this);
    layout->addWidget(m_backButton);
    registerInteractive(m_backButton);

    connect(m_connectButton, &QPushButton::clicked, this, [this]() {
        freezeForActivation();

        if (m_mode == Mode::MetadataDriven) {
            emit connectRequested(
                m_hostLabel->text(),
                m_portLabel->text(),
                m_usernameEdit->isVisible() ? m_usernameEdit->text() : QString(),
                m_passwordEdit->isVisible() ? m_passwordEdit->text() : QString()
            );
        } else {
            emit connectRequested(
                m_manualHostEdit->text(),
                m_manualPortEdit->text(),
                QString(),
                QString()
            );
        }
    });

    connect(m_manualButton, &QPushButton::clicked,
            this, &UnifiedConnectScreen::manualConnectRequested);

    connect(m_backButton, &QPushButton::clicked,
            this, &UnifiedConnectScreen::backRequested);

    configureForMetadata("", "", "", false);
}

void UnifiedConnectScreen::configureForMetadata(const QString& serverName,
                                                const QString& host,
                                                const QString& port,
                                                bool accountBased)
{
    m_mode = Mode::MetadataDriven;

    m_serverNameLabel->setText(serverName);
    m_hostLabel->setText(host);
    m_portLabel->setText(port);

    m_usernameEdit->setVisible(accountBased);
    m_passwordEdit->setVisible(accountBased);

    m_manualHostEdit->hide();
    m_manualPortEdit->hide();

    m_serverNameLabel->show();
    m_hostLabel->show();
    m_portLabel->show();
}

void UnifiedConnectScreen::configureForManual()
{
    m_mode = Mode::Manual;

    m_serverNameLabel->hide();
    m_hostLabel->hide();
    m_portLabel->hide();
    m_usernameEdit->hide();
    m_passwordEdit->hide();

    m_manualHostEdit->show();
    m_manualPortEdit->show();
}
