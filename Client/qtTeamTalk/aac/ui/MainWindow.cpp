#include "MainWindow.h"
#include "AACTitleBar.h"
#include "AACScreenBase.h"

#include "ui/UnifiedConnectScreen.h"
#include "ui/InChannelScreen.h"

#include "aac/ui/AACMainScreen.h"
#include "aac/ui/AACKeyboardScreen.h"
#include "aac/ui/AACSymbolGridScreen.h"
#include "aac/ui/AACSettingsScreen.h"
#include "aac/ui/AACSpeechSettingsScreen.h"

#include "ui/AppSettingsScreen.h"

#include "aac/AACFramework.h"
#include "backend/BackendAdapter.h"

#include "aac_server_discovery.h"
#include "aac_server_discovery_model.h"
#include "serverservice.h"

#include <QCloseEvent>

MainWindow::MainWindow(AACAccessibilityManager* aac,
                       BackendAdapter* backend,
                       AACServerDiscovery* discovery,
                       ServerService* serverService,
                       QWidget* parent)
    : QMainWindow(parent),
      m_aac(aac),
      m_backend(backend),
      m_discovery(discovery),
      m_serverService(serverService),
      m_stack(new QStackedWidget(this))
{
m_central = new QWidget(this);
setCentralWidget(m_central);

m_layout = new QVBoxLayout(m_central);
m_layout->setContentsMargins(0,0,0,0);
m_layout->setSpacing(0);

m_titleBar = new AACTitleBar(this);

m_layout->addWidget(m_titleBar);
m_layout->addWidget(m_stack);

createScreens();
wireNavigation();
showScreen(Screen_UnifiedConnect);
}

MainWindow::~MainWindow() = default;

void MainWindow::createScreens()
{
    m_unifiedConnectScreen = new UnifiedConnectScreen(m_aac, this);
    m_stack->addWidget(m_unifiedConnectScreen);

    m_inChannelScreen = new InChannelScreen(m_aac, m_backend, this);
    m_stack->addWidget(m_inChannelScreen);

    m_aacMainScreen = new AACMainScreen(m_aac, this);
    m_stack->addWidget(m_aacMainScreen);

    m_aacKeyboardScreen = new AACKeyboardScreen(m_aac, this);
    m_stack->addWidget(m_aacKeyboardScreen);

    m_aacSymbolGridScreen = new AACSymbolGridScreen(m_aac, this);
    m_stack->addWidget(m_aacSymbolGridScreen);

    m_settingsScreen = new AACSettingsScreen(m_aac, this);
    m_stack->addWidget(m_settingsScreen);

    m_speechSettingsScreen = new AACSpeechSettingsScreen(m_aac, this);
    m_stack->addWidget(m_speechSettingsScreen);

    m_appSettingsScreen = new AppSettingsScreen(m_aac, this);
    m_stack->addWidget(m_appSettingsScreen);
}

void MainWindow::wireNavigation()
{
    //
    // CONNECT
    //
    connect(m_unifiedConnectScreen, &UnifiedConnectScreen::connectRequested,
            this, [this](const QString& host,
                         const QString& port,
                         const QString& username,
                         const QString& password) {

        bool ok = false;
        int portNum = port.toInt(&ok);
        if (!ok) {
            m_unifiedConnectScreen->setErrorMessage(tr("Invalid port"));
            return;
        }

        m_unifiedConnectScreen->setErrorMessage(QString());
        m_backend->connectToServer(host, portNum, username, password);
    });

    connect(m_unifiedConnectScreen, &UnifiedConnectScreen::manualConnectRequested,
            this, [this] {
        m_unifiedConnectScreen->setErrorMessage(QString());
        m_unifiedConnectScreen->configureForManual();
    });

    connect(m_unifiedConnectScreen, &UnifiedConnectScreen::backRequested,
            this, [this] {
        m_unifiedConnectScreen->setErrorMessage(QString());
        m_unifiedConnectScreen->configureForMetadata("", "", "", false);
    });

    //
    // PUBLIC SERVERS
    //
    if (m_serverService) {
        connect(m_unifiedConnectScreen, &UnifiedConnectScreen::publicServersRequested,
                this, [this] {
            m_unifiedConnectScreen->setErrorMessage(QString());
            m_serverService->fetchPublicServers(true);
        });

        connect(m_serverService, &ServerService::publicServersUpdated,
                this, [this](const QList<ServerInfo>& servers) {

            QVector<UnifiedServerEntry> entries;
            entries.reserve(servers.size());

            for (const auto& s : servers) {
                UnifiedServerEntry e;
                e.name = s.name;
                e.host = s.host;
                e.port = QString::number(s.port);
                e.accountBased = false;
                entries.push_back(e);
            }

            m_unifiedConnectScreen->setErrorMessage(QString());
            m_unifiedConnectScreen->setServerList(entries);
        });
    }

    //
    // RECENT SERVERS
    //
    if (m_serverService) {
        connect(m_unifiedConnectScreen, &UnifiedConnectScreen::recentServersRequested,
                this, [this] {

            m_unifiedConnectScreen->setErrorMessage(QString());

            const QList<ServerInfo> recents = m_serverService->loadLatestHosts();

            QVector<UnifiedServerEntry> entries;
            entries.reserve(recents.size());

            for (const auto& s : recents) {
                UnifiedServerEntry e;
                e.name = s.name;
                e.host = s.host;
                e.port = QString::number(s.port);
                e.accountBased = false;
                entries.push_back(e);
            }

            m_unifiedConnectScreen->setServerList(entries);
        });
    }

    //
    // JOIN CODE
    //
    if (m_serverService) {
        connect(m_unifiedConnectScreen, &UnifiedConnectScreen::joinCodeEntered,
                this, [this](const QString& code) {
            m_unifiedConnectScreen->setErrorMessage(QString());
            if (!code.trimmed().isEmpty())
                m_serverService->resolveJoinCode(code);
        });

        connect(m_serverService, &ServerService::joinCodeResolved,
                this, [this](const ServerInfo& s) {

            m_unifiedConnectScreen->setErrorMessage(QString());
            m_unifiedConnectScreen->configureForMetadata(
                s.name,
                s.host,
                QString::number(s.port),
                false
            );
        });

        connect(m_serverService, &ServerService::joinCodeFailed,
                this, [this](const QString& reason) {
            m_unifiedConnectScreen->setErrorMessage(reason);
        });
    }

    //
    // LAN DISCOVERY
    //
    if (m_discovery) {
        connect(m_unifiedConnectScreen, &UnifiedConnectScreen::lanServersRequested,
                this, [this] {
            m_unifiedConnectScreen->setErrorMessage(QString());
            m_discovery->startDiscovery();
        });

        connect(m_discovery, &AACServerDiscovery::isScanningChanged,
                m_unifiedConnectScreen, &UnifiedConnectScreen::onScanningChanged);

        connect(m_discovery, &AACServerDiscovery::lastUpdatedChanged,
                m_unifiedConnectScreen, &UnifiedConnectScreen::onLastUpdatedChanged);

        connect(m_discovery, &AACServerDiscovery::serversUpdated,
                this, [this](const QVector<ServerInfo>& servers) {

            QVector<UnifiedServerEntry> entries;
            entries.reserve(servers.size());

            for (const auto& s : servers) {
                UnifiedServerEntry e;
                e.name = s.name;
                e.host = s.host;
                e.port = QString::number(s.port);
                e.accountBased = false; // S1
                entries.push_back(e);
            }

            m_unifiedConnectScreen->setErrorMessage(QString());
            m_unifiedConnectScreen->setServerList(entries);
        });
    }

    //
    // QR SCAN
    //
    connect(m_unifiedConnectScreen, &UnifiedConnectScreen::qrScanRequested,
            this, [this] {
        m_unifiedConnectScreen->setErrorMessage(QString());
        // QR scan UI would go here
    });

    //
    // IN CHANNEL SCREEN
    //
    connect(m_inChannelScreen, &InChannelScreen::leaveChannelRequested,
            this, [this] {
        m_backend->leaveChannel();
        showScreen(Screen_UnifiedConnect);
    });

    connect(m_inChannelScreen, &InChannelScreen::transmitToggled,
            this, [this](bool enabled) {
        m_backend->setTransmit(enabled);
    });

    //
    // AAC MAIN SCREEN
    //
    connect(m_aacMainScreen, &AACMainScreen::textCommitted,
            this, [this](const QString& text) {
        m_backend->sendTextMessage(text);
        m_backend->speak(text);
    });

    //
    // SYMBOL GRID → AAC MAIN
    //
    connect(m_aacSymbolGridScreen, &AACSymbolGridScreen::symbolActivated,
            m_aacMainScreen, &AACMainScreen::appendWord);

    //
    // SETTINGS
    //
    connect(m_settingsScreen, &AACSettingsScreen::backRequested,
            this, [this] { showScreen(Screen_UnifiedConnect); });

    connect(m_speechSettingsScreen, &AACSpeechSettingsScreen::backRequested,
            this, [this] { showScreen(Screen_Settings); });

    connect(m_appSettingsScreen, &AppSettingsScreen::backRequested,
            this, [this] { showScreen(Screen_UnifiedConnect); });
}

void MainWindow::showScreen(ScreenId id)
{
    QWidget *screen = m_stack->widget(static_cast<int>(id));
    m_stack->setCurrentWidget(screen);

    // Update title bar BEFORE AAC focus logic
    if (auto *base = qobject_cast<AACScreenBase*>(screen)) {
        connect(base, &AACScreenBase::requestTitleChange,
                m_titleBar, &AACTitleBar::setTitle,
                Qt::UniqueConnection);

        base->emitInitialTitle();
    }

    applyInitialFocus(id);
}
void MainWindow::applyInitialFocus(ScreenId id)
{
    Q_UNUSED(id);

    if (auto* aacScreen = currentAACScreen())
        m_aac->setActiveScreen(aacScreen);
}

AACScreenAdapter* MainWindow::currentAACScreen() const
{
    return qobject_cast<AACScreenAdapter*>(m_stack->currentWidget());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_backend->shutdown();
    QMainWindow::closeEvent(event);
}
