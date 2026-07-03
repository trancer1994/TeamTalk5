#include "ConnectScreen.h"
#include "AACKeyButton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

ConnectScreen::ConnectScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("Connect to Server"));

    auto* main = new QVBoxLayout(this);

    // Host
    auto* hostLabel = new QLabel(tr("Server address"), this);
    m_hostEdit = new QLineEdit(this);
    main->addWidget(hostLabel);
    main->addWidget(m_hostEdit);

    // Port
    auto* portLabel = new QLabel(tr("Port"), this);
    m_portEdit = new QLineEdit(this);
    main->addWidget(portLabel);
    main->addWidget(m_portEdit);

    // Username
    auto* userLabel = new QLabel(tr("Username"), this);
    m_userEdit = new QLineEdit(this);
    main->addWidget(userLabel);
    main->addWidget(m_userEdit);

    // Password
    auto* passLabel = new QLabel(tr("Password"), this);
    m_passEdit = new QLineEdit(this);
    m_passEdit->setEchoMode(QLineEdit::Password);
    main->addWidget(passLabel);
    main->addWidget(m_passEdit);

    // Buttons
    auto* row = new QHBoxLayout();
    m_backBtn      = new AACKeyButton(aac, tr("Back"), this);
    m_backBtn->setDeepWell(true);
    m_connectBtn   = new AACKeyButton(aac, tr("Connect"), this);
    m_reconnectBtn = new AACKeyButton(aac, tr("Reconnect"), this);
    row->addWidget(m_backBtn);
    row->addWidget(m_connectBtn);
    row->addWidget(m_reconnectBtn);
    row->setSpacing(12);
    main->addLayout(row);

    // Prefill from last successful connection
    if (m_aac) {
        m_hostEdit->setText(m_aac->lastHost());
        if (m_aac->lastPort() != 0)
            m_portEdit->setText(QString::number(m_aac->lastPort()));
        m_userEdit->setText(m_aac->lastUsername());
        m_passEdit->setText(m_aac->lastPassword());
    }

    connect(m_reconnectBtn, &AACKeyButton::activated, this, [this]() {
        emit reconnectRequested();
    });

    // Reconnect overlay
    m_reconnectOverlay = new QWidget(this);
    m_reconnectOverlay->setStyleSheet("background: rgba(0, 0, 0, 128);");
    m_reconnectOverlay->setVisible(false);

    auto* overlayLayout = new QVBoxLayout(m_reconnectOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);

    m_reconnectLabel = new QLabel(tr("Reconnecting…"), m_reconnectOverlay);
    m_reconnectLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    overlayLayout->addWidget(m_reconnectLabel);

    m_reconnectOverlay->setGeometry(rect());
    m_reconnectOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_reconnectOverlay->raise();
    m_reconnectOverlay->setFocusPolicy(Qt::StrongFocus);

    // Back button
    connect(m_backBtn, &AACKeyButton::activated, this, [this]() {
        speak(tr("Back"));
        emit backRequested();
        emit reconnectCancelled();
    });

    // Connect button
    connect(m_connectBtn, &AACKeyButton::activated, this, [this]() {
        emit reconnectCancelled();

        ServerInfo info;
        info.host     = m_hostEdit->text();
        info.port     = m_portEdit->text().toUShort();
        info.username = m_userEdit->text();
        info.password = m_passEdit->text();

        emit connectRequested(info);
    });

    // Any manual edit cancels reconnect
    auto cancelReconnectOnEdit = [this]() {
        emit reconnectCancelled();
    };
    connect(m_hostEdit, &QLineEdit::textEdited, this, cancelReconnectOnEdit);
    connect(m_portEdit, &QLineEdit::textEdited, this, cancelReconnectOnEdit);
    connect(m_userEdit, &QLineEdit::textEdited, this, cancelReconnectOnEdit);
    connect(m_passEdit, &QLineEdit::textEdited, this, cancelReconnectOnEdit);

    // Mode cycling — no reconnect cancellation, no connection earcons
    connect(this, &ConnectScreen::cycleNextMode, this, [this]() {
        speak(tr("Next mode"));
    if (m_aac && m_aac->earcons())
        m_aac->earcons()->play(Earcon::Navigate);
    });

    connect(this, &ConnectScreen::cyclePrevMode, this, [this]() {
        speak(tr("Previous mode"));
    if (m_aac && m_aac->earcons())
        m_aac->earcons()->play(Earcon::Navigate);
    });

    setLayout(main);
}
QString ConnectScreen::contextualHelp() const
{
    return tr("Connect. "
               "Press F5 to try connecting again. "
               "Press Escape to go back. "
               "Press F1 for AAC Settings. "
               "Press F2 for Speech Settings. "
               "Press F3 for App Settings.");
}
//
// Set mode: MetadataDriven or Manual
//
void ConnectScreen::setMode(ConnectMode mode)
{
    m_mode = mode;

    if (mode == ConnectMode::MetadataDriven) {
        m_hostEdit->setReadOnly(true);
        m_portEdit->setReadOnly(true);
        speak(tr("Metadata mode"));
    } else {
        m_hostEdit->setReadOnly(false);
        m_portEdit->setReadOnly(false);
        speak(tr("Manual mode"));
    }
}

//
// Pre-fill host/port when metadata-driven
//
void ConnectScreen::setMetadata(const QString& host, quint16 port)
{
    m_hostEdit->setText(host);
    m_portEdit->setText(QString::number(port));

    speak(tr("Server details loaded"));
}
void ConnectScreen::keyPressEvent(QKeyEvent* e)
{
    // ESC → Back
if (e->key() == Qt::Key_Escape) {
    speak(tr("Back"));
    emit backRequested();
    emit reconnectCancelled();
    return;
}

if (e->key() == Qt::Key_F5) {
    emit reconnectRequested();
    return;
}
    // Ctrl+Tab → next mode
    if ((e->modifiers() & Qt::ControlModifier) &&
        e->key() == Qt::Key_Tab &&
        !(e->modifiers() & Qt::ShiftModifier)) {
        emit cycleNextMode();
        return;
    }

    // Ctrl+Shift+Tab → previous mode
    if ((e->modifiers() & Qt::ControlModifier) &&
        (e->key() == Qt::Key_Backtab ||
         (e->key() == Qt::Key_Tab && (e->modifiers() & Qt::ShiftModifier)))) {
        emit cyclePrevMode();
        return;
    }

    // Home → focus first field
    if (e->key() == Qt::Key_Home) {
        if (m_hostEdit)
            m_hostEdit->setFocus();
        return;
    }

    // End → focus Connect button
    if (e->key() == Qt::Key_End) {
        if (m_connectBtn)
            m_connectBtn->setFocus();
        return;
    }

    AACScreenBase::keyPressEvent(e);
}
void ConnectScreen::speak(const QString& text)
{
    if (!m_aac || !m_aac->speechEngine())
        return;

    m_aac->speechEngine()->speak(text);
}
void ConnectScreen::showReconnectSpinner()
{
    setUiEnabled(false);
    if (m_reconnectOverlay) {
        m_reconnectOverlay->setVisible(true);
        m_reconnectOverlay->raise();
        m_reconnectOverlay->setFocus();
m_reconnectBtn->setEnabled(false);
        speak(tr("Reconnecting"));
m_aac->earcons()->play(Earcon::ReconnectStart);
    }
}

void ConnectScreen::hideReconnectSpinner()
{
    if (m_reconnectOverlay)
        m_reconnectOverlay->setVisible(false);

    setUiEnabled(true);
    m_reconnectBtn->setEnabled(true);
    if (m_reconnectBtn)
        m_reconnectBtn->setFocus();
}
void ConnectScreen::resizeEvent(QResizeEvent* event)
{
    AACScreenBase::resizeEvent(event);
    if (m_reconnectOverlay)
        m_reconnectOverlay->setGeometry(rect());
}
void ConnectScreen::setUiEnabled(bool enabled)
{
    m_hostEdit->setEnabled(enabled);
    m_portEdit->setEnabled(enabled);
    m_userEdit->setEnabled(enabled);
    m_passEdit->setEnabled(enabled);
    m_connectBtn->setEnabled(enabled);
    m_backBtn->setEnabled(enabled);
}
