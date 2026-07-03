#include "AACServerDiscoveryScreen.h"
#include <QVBoxLayout>
#include <QLabel>

AACServerDiscoveryScreen::AACServerDiscoveryScreen(AACAccessibilityManager* aac,
                                                   QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("Find Servers"));

    auto* main = new QVBoxLayout(this);
    main->setSpacing(12);

    //
    // LAN
    //
    m_lanBtn = new AACKeyButton(aac, tr("LAN Servers"), this);
    m_lanBtn->setDeepWell(true);
    connect(m_lanBtn, &AACKeyButton::activated,
            this, [this]() { emit lanRequested(); });
    main->addWidget(m_lanBtn);

    //
    // Public
    //
    m_publicBtn = new AACKeyButton(aac, tr("Public Servers"), this);
    m_publicBtn->setDeepWell(true);
    connect(m_publicBtn, &AACKeyButton::activated,
            this, [this]() { emit publicServersRequested(); });
    main->addWidget(m_publicBtn);

    //
    // Recent
    //
    m_recentBtn = new AACKeyButton(aac, tr("Recent Servers"), this);
    m_recentBtn->setDeepWell(true);
    connect(m_recentBtn, &AACKeyButton::activated,
            this, [this]() { emit recentServersRequested(); });
    main->addWidget(m_recentBtn);

    //
    // Join Code
    //
    m_joinCodeBtn = new AACKeyButton(aac, tr("Join Code"), this);
    m_joinCodeBtn->setDeepWell(true);
    connect(m_joinCodeBtn, &AACKeyButton::activated,
            this, [this]() { emit joinCodeRequested(); });
    main->addWidget(m_joinCodeBtn);

    //
    // QR Scan
    //
    m_qrBtn = new AACKeyButton(aac, tr("Scan QR Code"), this);
    m_qrBtn->setDeepWell(true);
    connect(m_qrBtn, &AACKeyButton::activated,
            this, [this]() { emit qrScanRequested(); });
    main->addWidget(m_qrBtn);

    //
    // Deep Link (NEW)
    //
    m_deepLinkBtn = new AACKeyButton(aac, tr("Deep Link"), this);
    m_deepLinkBtn->setDeepWell(true);
    connect(m_deepLinkBtn, &AACKeyButton::activated,
            this, [this]() { emit deepLinkRequested(); });
    main->addWidget(m_deepLinkBtn);

    //
    // Back
    //
    m_backBtn = new AACKeyButton(aac, tr("Back"), this);
    m_backBtn->setDeepWell(true);
    connect(m_backBtn, &AACKeyButton::activated,
            this, [this]() { emit backRequested(); });
    main->addWidget(m_backBtn);

    setLayout(main);
}

QString AACServerDiscoveryScreen::contextualHelp() const
{
    return tr("Server discovery. "
              "Press LAN Servers to find servers on your network. "
              "Press Public Servers to browse public servers. "
              "Press Recent Servers to view your history. "
              "Press Join Code to enter a code. "
              "Press Scan QR Code to scan a TeamTalk QR code. "
              "Press Deep Link to enter a TeamTalk link. "
              "Press Escape to go back. "
              "Press F1 for AAC Settings. "
              "Press F2 for Speech Settings. "
              "Press F3 for App Settings. "
              "Press F12 for help.");
}

void AACServerDiscoveryScreen::keyPressEvent(QKeyEvent* e)
{
    const AACModeFlags modes = m_aac->modes();
    const bool allowShortcuts = (!modes.scanning && !modes.dwell);

    if (allowShortcuts && e->key() == Qt::Key_F5) {
        emit qrScanRequested();
        return;
    }

    AACScreenBase::keyPressEvent(e);
}
