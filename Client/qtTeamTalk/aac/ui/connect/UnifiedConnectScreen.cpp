#include "UnifiedConnectScreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>

UnifiedConnectScreen::UnifiedConnectScreen(AACAccessibilityManager* aac,
                                           QWidget* parent)
    : AACScreenBase(parent)
{
    setScreenTitle("Connect to Server");
    buildUi();
}

QString UnifiedConnectScreen::makeKey(const QString& host,
                                      const QString& port) const
{
    return host + ":" + port;
}

void UnifiedConnectScreen::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);
    root->setContentsMargins(16, 16, 16, 16);

    //
    // 1. Source buttons (top strip)
    //
    {
        auto* row = new QHBoxLayout;
        row->setSpacing(8);

        m_publicButton   = new QPushButton(tr("Public"), this);
        m_lanButton      = new QPushButton(tr("LAN"), this);
        m_recentButton   = new QPushButton(tr("Recent"), this);
        m_joinCodeButton = new QPushButton(tr("Join code"), this);
        m_qrButton       = new QPushButton(tr("Scan QR"), this);

        row->addWidget(m_publicButton);
        row->addWidget(m_lanButton);
        row->addWidget(m_recentButton);
        row->addWidget(m_joinCodeButton);
        row->addWidget(m_qrButton);

        root->addLayout(row);

        registerInteractive(m_publicButton);
        registerInteractive(m_lanButton);
        registerInteractive(m_recentButton);
        registerInteractive(m_joinCodeButton);
        registerInteractive(m_qrButton);
    }

    //
    // 2. Discovery status strip
    //
    {
        auto* row = new QHBoxLayout;
        row->setSpacing(8);

        m_scanningLabel = new QLabel(tr("Idle"), this);
        m_scanAgainButton = new QPushButton(tr("Scan Again"), this);

        row->addWidget(m_scanningLabel);
        row->addStretch();
        row->addWidget(m_scanAgainButton);

        root->addLayout(row);

        registerInteractive(m_scanAgainButton);

        connect(m_scanAgainButton, &QPushButton::clicked,
                this, &UnifiedConnectScreen::scanAgainRequested);
    }

    //
    // 2b. Error strip (static, AAC-friendly)
    //
    {
        m_errorLabel = new QLabel(this);
        m_errorLabel->setVisible(false);
        // Optional: subtle style, not shouting
        QPalette pal = m_errorLabel->palette();
        pal.setColor(QPalette::WindowText, QColor(Qt::red));
        m_errorLabel->setPalette(pal);

        root->addWidget(m_errorLabel);
        registerInteractive(m_errorLabel);
    }

    //
    // 3. Server selection block
    //
    {
        auto* block = new QVBoxLayout;
        block->setSpacing(6);

        m_serverList = new QListWidget(this);
        m_serverList->setMinimumHeight(160);
        block->addWidget(m_serverList);
        registerInteractive(m_serverList);

        // "No servers found" label (static, AAC-friendly)
        m_emptyLabel = new QLabel(tr("No servers found"), this);
        m_emptyLabel->setVisible(false);
        block->addWidget(m_emptyLabel);
        registerInteractive(m_emptyLabel);

        m_joinCodeEdit = new QLineEdit(this);
        m_joinCodeEdit->setPlaceholderText(tr("Enter join code"));
        block->addWidget(m_joinCodeEdit);
        registerInteractive(m_joinCodeEdit);

        root->addLayout(block);
    }

    //
    // 4. Connection details block
    //
    {
        auto* block = new QVBoxLayout;
        block->setSpacing(4);

        m_serverNameLabel = new QLabel(this);
        block->addWidget(m_serverNameLabel);
        registerInteractive(m_serverNameLabel);

        m_hostLabel = new QLabel(this);
        block->addWidget(m_hostLabel);
        registerInteractive(m_hostLabel);

        m_portLabel = new QLabel(this);
        block->addWidget(m_portLabel);
        registerInteractive(m_portLabel);

        m_usernameEdit = new QLineEdit(this);
        block->addWidget(m_usernameEdit);
        registerInteractive(m_usernameEdit);

        m_passwordEdit = new QLineEdit(this);
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        block->addWidget(m_passwordEdit);
        registerInteractive(m_passwordEdit);

        m_manualHostEdit = new QLineEdit(this);
        m_manualPortEdit = new QLineEdit(this);
        block->addWidget(m_manualHostEdit);
        block->addWidget(m_manualPortEdit);
        registerInteractive(m_manualHostEdit);
        registerInteractive(m_manualPortEdit);

        root->addLayout(block);
    }

    //
    // 5. Bottom action strip
    //
    {
        auto* row = new QHBoxLayout;
        row->setSpacing(12);

        m_connectButton = new QPushButton(tr("Connect"), this);
        m_manualButton  = new QPushButton(tr("Manual Connect"), this);
        m_backButton    = new QPushButton(tr("Back"), this);

        row->addWidget(m_connectButton);
        row->addWidget(m_manualButton);
        row->addStretch();
        row->addWidget(m_backButton);

        root->addLayout(row);

        registerInteractive(m_connectButton, true);
        registerInteractive(m_manualButton);
        registerInteractive(m_backButton);
    }

    //
    // Behaviour
    //
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
                m_usernameEdit->text(),
                m_passwordEdit->text()
            );
        }
    });

    connect(m_manualButton, &QPushButton::clicked,
            this, &UnifiedConnectScreen::manualConnectRequested);

    connect(m_backButton, &QPushButton::clicked,
            this, &UnifiedConnectScreen::backRequested);

    connect(m_publicButton, &QPushButton::clicked,
            this, &UnifiedConnectScreen::publicServersRequested);

    connect(m_lanButton, &QPushButton::clicked,
            this, &UnifiedConnectScreen::lanServersRequested);

    connect(m_recentButton, &QPushButton::clicked,
            this, &UnifiedConnectScreen::recentServersRequested);

    connect(m_joinCodeButton, &QPushButton::clicked, this, [this]() {
        emit joinCodeEntered(m_joinCodeEdit->text());
    });

    connect(m_qrButton, &QPushButton::clicked,
            this, &UnifiedConnectScreen::qrScanRequested);

    connect(m_serverList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        const int row = m_serverList->row(item);
        if (row < 0 || row >= m_servers.size())
            return;

        const auto& s = m_servers[row];
        configureForMetadata(s.name, s.host, s.port, s.accountBased);

        // Highlight persistence: remember last selected key
        m_lastSelectedKey = makeKey(s.host, s.port);
    });

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

    m_manualHostEdit->show();
    m_manualPortEdit->show();

    m_usernameEdit->show();
    m_passwordEdit->show();
}

void UnifiedConnectScreen::setServerList(const QVector<UnifiedServerEntry>& servers)
{
    // Preserve current selection key (if any)
    QString currentKey;
    if (m_serverList->currentRow() >= 0 &&
        m_serverList->currentRow() < m_servers.size()) {
        const auto& s = m_servers[m_serverList->currentRow()];
        currentKey = makeKey(s.host, s.port);
    }
    if (!m_lastSelectedKey.isEmpty())
        currentKey = m_lastSelectedKey;

    m_servers = servers;
    m_serverList->clear();

    int rowToSelect = -1;

    for (int i = 0; i < m_servers.size(); ++i) {
        const auto& s = m_servers[i];
        auto* item = new QListWidgetItem(
            QStringLiteral("%1 (%2:%3)")
                .arg(s.name.isEmpty() ? s.host : s.name,
                     s.host,
                     s.port),
            m_serverList);
        item->setData(Qt::UserRole, s.accountBased);

        const QString key = makeKey(s.host, s.port);
        if (!currentKey.isEmpty() && key == currentKey)
            rowToSelect = i;
    }

    if (rowToSelect >= 0) {
        m_serverList->setCurrentRow(rowToSelect);
    }

    updateEmptyLabel();
}

void UnifiedConnectScreen::updateEmptyLabel()
{
    const bool empty = m_servers.isEmpty();
    m_emptyLabel->setVisible(empty);
}

void UnifiedConnectScreen::onScanningChanged(bool scanning)
{
    if (scanning) {
        m_scanningLabel->setText(tr("Scanning…"));
        m_scanAgainButton->setEnabled(false);

        // Freeze interactive elements during scan
        m_serverList->setEnabled(false);
        m_joinCodeEdit->setEnabled(false);

        m_publicButton->setEnabled(false);
        m_lanButton->setEnabled(false);
        m_recentButton->setEnabled(false);
        m_joinCodeButton->setEnabled(false);
        m_qrButton->setEnabled(false);

        m_connectButton->setEnabled(false);
        m_manualButton->setEnabled(false);
        // Back remains enabled for escape
    } else {
        m_scanningLabel->setText(tr("Idle"));
        m_scanAgainButton->setEnabled(true);

        m_serverList->setEnabled(true);
        m_joinCodeEdit->setEnabled(true);

        m_publicButton->setEnabled(true);
        m_lanButton->setEnabled(true);
        m_recentButton->setEnabled(true);
        m_joinCodeButton->setEnabled(true);
        m_qrButton->setEnabled(true);

        m_connectButton->setEnabled(true);
        m_manualButton->setEnabled(true);
    }
}

void UnifiedConnectScreen::onLastUpdatedChanged(const QDateTime& dt)
{
    m_scanningLabel->setText(tr("Last updated: %1")
                             .arg(dt.toString("HH:mm:ss")));
}

void UnifiedConnectScreen::setErrorMessage(const QString& message)
{
    if (message.isEmpty()) {
        m_errorLabel->setVisible(false);
        m_errorLabel->clear();
    } else {
        m_errorLabel->setText(message);
        m_errorLabel->setVisible(true);
    }
}
