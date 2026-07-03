#include "QRScannerScreen.h"
#include "AACKeyButton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

QRScannerScreen::QRScannerScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("Scan QR Code"));

    auto* main = new QVBoxLayout(this);

    // Camera preview placeholder
    m_previewLabel = new QLabel(tr("Camera preview would appear here"), this);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("background: #222; color: #ccc; padding: 40px;");
    main->addWidget(m_previewLabel);

    // Buttons
    auto* row = new QHBoxLayout();
    m_backBtn = new AACKeyButton(aac, tr("Back"), this);
    m_backBtn->setDeepWell(true);

    m_scanBtn = new AACKeyButton(aac, tr("Scan now"), this);
    m_scanBtn->setDeepWell(true);

    row->addWidget(m_backBtn);
    row->addWidget(m_scanBtn);
    row->setSpacing(12);

    main->addLayout(row);

    // Back
    connect(m_backBtn, &AACKeyButton::activated, this, [this]() {
        emit backRequested();
    });

    // Scan (placeholder)
    connect(m_scanBtn, &AACKeyButton::activated,
            this, &QRScannerScreen::onFakeScan);

    setLayout(main);
}

void QRScannerScreen::onFakeScan()
{
    // Placeholder: simulate a QR payload
    QString payload = tr("teamtalk://join?host=example.com&tcp=10333&udp=10333");

    if (m_aac && m_aac->speechEngine())
        m_aac->speechEngine()->speak(tr("QR code detected"));

    emit qrPayloadDetected(payload);
}

QString QRScannerScreen::contextualHelp() const
{
    return tr("QR Scanner. "
              "Point the camera at a TeamTalk QR code. "
              "Press Scan Now to capture it. "
              "Press F5 to scan the QR code. "
              "Press Escape to go back. "
              "Press F1 for AAC Settings. "
              "Press F2 for Speech Settings. "
              "Press F3 for App Settings. "
              "Press F12 for help.");
}
void QRScannerScreen::keyPressEvent(QKeyEvent* e)
{
    const AACModeFlags modes = m_aac->modes();

    // Only allow shortcuts for Touch/Click and Keyboard users
    const bool allowShortcuts = (!modes.scanning && !modes.dwell);

    if (allowShortcuts) {

        if (e->key() == Qt::Key_F5) {
            onFakeScan();
            return;
        }

        if (e->key() == Qt::Key_Escape) {
            emit backRequested();
            return;
        }
    }

    AACScreenBase::keyPressEvent(e);
}
