#include "ConnectionHubScreen.h"
#include <QVBoxLayout>

ConnectionHubScreen::ConnectionHubScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("Connection Options"));

    auto* layout = new QVBoxLayout(this);

    // 1. Reconnect (only if available)
    if (m_aac && !m_aac->lastHost().isEmpty()) {
        auto* reconnectBtn = new AACKeyButton(aac, tr("Reconnect to last server"), this);
        reconnectBtn->setDeepWell(true);
        connect(reconnectBtn, &AACKeyButton::activated, this, [this]() {
            emit reconnectRequested();
        });
        layout->addWidget(reconnectBtn);
    }

    // 2. Manual connection
    auto* manualBtn = new AACKeyButton(aac, tr("Connect manually"), this);
    manualBtn->setDeepWell(true);
    connect(manualBtn, &AACKeyButton::activated, this, [this]() {
        emit connectManualRequested();
    });
    layout->addWidget(manualBtn);

    // 3. LAN discovery
    auto* findBtn = new AACKeyButton(aac, tr("Find servers on the network"), this);
    findBtn->setDeepWell(true);
    connect(findBtn, &AACKeyButton::activated, this, [this]() {
        emit findServersRequested();
    });
    layout->addWidget(findBtn);

    // 4. Public servers
    auto* publicBtn = new AACKeyButton(aac, tr("Browse public servers"), this);
    publicBtn->setDeepWell(true);
    connect(publicBtn, &AACKeyButton::activated, this, [this]() {
        emit publicServersRequested();
    });
    layout->addWidget(publicBtn);

    // 5. Join code
    auto* joinBtn = new AACKeyButton(aac, tr("Enter join code"), this);
    joinBtn->setDeepWell(true);
    connect(joinBtn, &AACKeyButton::activated, this, [this]() {
        emit joinCodeRequested();
    });
    layout->addWidget(joinBtn);

    // 6. QR scan
    auto* qrBtn = new AACKeyButton(aac, tr("Scan QR code"), this);
    qrBtn->setDeepWell(true);
    connect(qrBtn, &AACKeyButton::activated, this, [this]() {
        emit qrScanRequested();
    });
    layout->addWidget(qrBtn);

    // 7. Back
    auto* backBtn = new AACKeyButton(aac, tr("Back"), this);
    backBtn->setDeepWell(true);
    connect(backBtn, &AACKeyButton::activated, this, [this]() {
        emit backRequested();
    });
    layout->addWidget(backBtn);

    setLayout(layout);
}

QString ConnectionHubScreen::contextualHelp() const
{
    return tr(
        "Connection Hub. "
        "Choose how you want to connect. "
        "Reconnect to the last server, connect manually, find servers on the network, "
        "browse public servers, enter a join code, or scan a QR code. "
        "Press Escape to go back. "
        "Press F1 for AAC Settings. "
        "Press F2 for Speech Settings. "
        "Press F3 for App Settings. "
        "Press F12 for help."
    );
}
