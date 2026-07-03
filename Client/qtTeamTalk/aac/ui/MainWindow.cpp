#include "MainWindow.h"
#include "AACTitleBar.h"
#include "AACScreenBase.h"

#include "ui/ConnectScreen.h"
#include "ui/InChannelScreen.h"

#include "aac/ui/AACMainScreen.h"
#include "aac/ui/AACKeyboardScreen.h"
#include "aac/ui/AACSymbolGridScreen.h"
#include "aac/ui/AACSettingsScreen.h"
#include "aac/ui/AACSpeechSettingsScreen.h"
#include "aac/ui/AACEarconRouter.h"

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
m_earcons = new AACEarconRouter(m_aac, this);
    m_central = new QWidget(this);
    setCentralWidget(m_central);

    m_layout = new QVBoxLayout(m_central);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);

    m_titleBar = new AACTitleBar(this);

    m_layout->addWidget(m_titleBar);
    m_layout->addWidget(m_stack);

    createScreens();
initHelpRegistry();
    wireNavigation();

    }
});
if (!m_aac->profile().isConfigured()) {
    showScreen(Screen_AACCommMethod);
} else {
    showScreen(Screen_ConnectionHub);
}
qApp->installEventFilter(this);

MainWindow::~MainWindow() = default;

void MainWindow::createScreens()
{
m_aacCommMethodScreen = new AACCommunicationMethodScreen(m_aac, this);
m_stack->addWidget(m_aacCommMethodScreen);

m_aacTypingMethodScreen = new AACTypingMethodScreen(m_aac, this);
m_stack->addWidget(m_aacTypingMethodScreen);

m_aacVocabularyScreen = new AACVocabularyScreen(m_aac, this);
m_stack->addWidget(m_aacVocabularyScreen);

m_aacSummaryScreen = new AACSetupSummaryScreen(m_aac, this);
m_stack->addWidget(m_aacSummaryScreen);

m_AACServerDiscoveryScreen = new AACServerDiscoveryScreen(m_aac, this);
m_stack->addWidget(m_AACServerDiscoveryScreen);

m_connectionHubScreen = new ConnectionHubScreen(m_aac, this);
m_stack->addWidget(m_connectionHubScreen);

    m_ConnectScreen = new ConnectScreen(m_aac, this);
    m_stack->addWidget(m_ConnectScreen);

connect(m_ConnectScreen, &ConnectScreen::cycleNextMode,
        this, [this]() { cycleConnectMode(+1); });

connect(m_ConnectScreen, &ConnectScreen::cyclePrevMode,
        this, [this]() { cycleConnectMode(-1); });

m_channelListScreen = new ChannelListScreen(m_aac, this);
m_stack->addWidget(m_channelListScreen);

m_channelPasswordScreen = new ChannelPasswordScreen(m_aac, this);
m_stack->addWidget(m_channelPasswordScreen);

    m_inChannelScreen = new InChannelScreen(m_aac, m_backend, this);
    m_stack->addWidget(m_inChannelScreen);

    m_aacMainScreen = new AACMainScreen(m_aac, this);
    m_stack->addWidget(m_aacMainScreen);

// ⭐ Unified AAC send pipeline
connect(m_aacMainScreen, &AACMainScreen::sendToChannelMessage,
        this, &MainWindow::onSendToChannel);

connect(m_aacMainScreen, &AACMainScreen::sendToUserMessage,
        this, &MainWindow::onSendToUser);

connect(m_aacMainScreen, &AACMainScreen::speakAACMessage,
        this, &MainWindow::onSpeakMessage);

    m_aacKeyboardScreen = new AACKeyboardScreen(m_aac, this);
    m_stack->addWidget(m_aacKeyboardScreen);

connect(m_aacKeyboardScreen, &AACKeyboardScreen::symbolSemantic,
        m_aac->predictionEngine(), &AACPredictionEngine::setSemanticContext);
connect(m_aac->predictionEngine(),
        &AACPredictionEngine::semanticContextChanged,
        m_aacKeyboardScreen,
        &AACKeyboardScreen::setSemanticHighlight);
connect(m_aac->predictionEngine(),
        &AACPredictionEngine::semanticContextChanged,
        m_aacMainScreen,
        &AACMainScreen::onSemanticContextChanged);

    m_aacSymbolGridScreen = new AACSymbolGridScreen(m_aac, this);
    m_stack->addWidget(m_aacSymbolGridScreen);

connect(m_aac->predictionEngine(),
        &AACPredictionEngine::semanticContextChanged,
        m_aacSymbolGridScreen,
        &AACSymbolGridScreen::setSemanticHighlight);

    m_settingsScreen = new AACSettingsScreen(m_aac, this);
    m_stack->addWidget(m_settingsScreen);

    m_speechSettingsScreen = new AACSpeechSettingsScreen(m_aac, this);
    m_stack->addWidget(m_speechSettingsScreen);

    m_appSettingsScreen = new AppSettingsScreen(m_aac, this);
    m_stack->addWidget(m_appSettingsScreen);
}

void MainWindow::initHelpRegistry()
{
    // AAC Communication Method
    m_helpRegistry[Screen_AACCommMethod].screenHelp =
        tr("Choose how you want to control the AAC system. Touch, dwell, or switch scanning.");

    // Typing Method
    m_helpRegistry[Screen_AACTypingMethod].screenHelp =
        tr("Choose how you want to type. Keyboard, symbol grid, or both.");

    // Vocabulary
    m_helpRegistry[Screen_AACVocabulary].screenHelp =
        tr("Choose your vocabulary set.");

    // Summary
    m_helpRegistry[Screen_AACSummary].screenHelp =
        tr("Review your choices. Press Enter to finish setup.");

    // Connection Hub
    m_helpRegistry[Screen_ConnectionHub].screenHelp =
        tr("Connect to a server. Choose LAN, public, recent, join code, or scan a QR code.");

    // Server Discovery
    m_helpRegistry[Screen_AACServerDiscovery].screenHelp =
        tr("Find servers on your network or online.");

    // Connect Screen
    m_helpRegistry[Screen_Connect].screenHelp =
        tr("Enter server details or choose from the list.");

    // Channel List
    m_helpRegistry[Screen_ChannelList].screenHelp =
        tr("Choose a channel to join.");

    // Channel Password
    m_helpRegistry[Screen_ChannelPassword].screenHelp =
        tr("Enter the channel password.");

    // In Channel
    m_helpRegistry[Screen_InChannel].screenHelp =
        tr("You are in a channel. Press F8 to talk, or use AAC to communicate.");

    // AAC Main
    m_helpRegistry[Screen_AACMain].screenHelp =
        tr("Compose your message. Press F4 for keyboard or F5 for symbol grid.");

    // Keyboard
    m_helpRegistry[Screen_AACKeyboard].screenHelp =
        tr("Type letters. Press F6 to speak. Press F7 to clear.");

// --- AACKeyboardScreen element help ---
m_helpRegistry[Screen_AACKeyboard].elementHelp["backspaceButton"] =
    tr("Delete the previous character.");
...
m_helpRegistry[Screen_AACKeyboard].elementHelp["key_a"] =
    tr("Letter A.");

    // Symbol Grid
    m_helpRegistry[Screen_AACSymbolGrid].screenHelp =
        tr("Choose a symbol to insert a word.");

    // Settings
    m_helpRegistry[Screen_Settings].screenHelp =
        tr("AAC settings. Change transmit mode and other options.");

    // Speech Settings
    m_helpRegistry[Screen_SpeechSettings].screenHelp =
        tr("Speech settings. Adjust voice, rate, and volume.");

    // App Settings
    m_helpRegistry[Screen_AppSettings].screenHelp =
        tr("Application settings.");
}
void MainWindow::cycleConnectMode(int delta)
{
    static QVector<ConnectMode> modes = {
        ConnectMode::Manual,
        ConnectMode::MetadataDriven
    };

    int idx = modes.indexOf(m_currentMode);
    if (idx < 0) idx = 0;

    idx = (idx + delta + modes.size()) % modes.size();
    m_currentMode = modes[idx];

    m_ConnectScreen->setMode(m_currentMode);
}
void MainWindow::wireNavigation()
{
//
// ONBOARDING FLOW
//

// 1. Communication method → Typing method
connect(m_aacCommMethodScreen, &AACCommunicationMethodScreen::communicationMethodChosen,
        this, [this](CommMethod method) {

    m_pendingProfile.commMethod = method;
    showScreen(Screen_AACTypingMethod);
});

// 2. Typing method → Vocabulary OR Summary
connect(m_aacTypingMethodScreen, &AACTypingMethodScreen::typingMethodChosen,
        this, [this](TypingMethod method) {

    m_pendingProfile.typingMethod = method;

    if (method == TypingMethod::SymbolGrid || method == TypingMethod::Both)
        showScreen(Screen_AACVocabulary);
    else
        showScreen(Screen_AACSummary);
});

// 3. Vocabulary → Summary
connect(m_aacVocabularyScreen, &AACVocabularyScreen::vocabularyChosen,
        this, [this](const QString& vocabId) {

    m_pendingProfile.vocabId = vocabId;
    showScreen(Screen_AACSummary);
});

// 4. Summary → Finish onboarding → Connect
connect(m_aacSummaryScreen, &AACSetupSummaryScreen::setupComplete,
        this, [this](const AACProfileConfig& cfg) {

    m_aac->profile().commMethod   = cfg.commMethod;
    m_aac->profile().typingMethod = cfg.typingMethod;
    m_aac->profile().vocabId      = cfg.vocabId;
    m_aac->profile().configured = true;

    // Apply to AACAccessibilityManager
    AACModeFlags modes = m_aac->modes();

    switch (cfg.commMethod) {
    case CommMethod::TouchClick:
        modes.dwell = false;
        modes.scanning = false;
        break;
    case CommMethod::GazeDwell:
        modes.dwell = true;
        modes.scanning = false;
        break;
    case CommMethod::SwitchScanning:
        modes.dwell = false;
        modes.scanning = true;
        break;
    }
modes.helpMode = cfg.helpMode;
m_aac->setModes(modes);

// ⭐ System‑priority announcements for mode changes
const AACModeFlags newModes = m_aac->modes();
if (m_aac->speechEngine() && !newModes.fatigueMode) {

    // Scanning
    m_aac->speechEngine()->speakNotification(
        newModes.scanning ? tr("Scanning enabled")
                          : tr("Scanning disabled"),
        SpeechPriority::System
    );

    // Dwell
    m_aac->speechEngine()->speakNotification(
        newModes.dwell ? tr("Dwell enabled")
                       : tr("Dwell disabled"),
        SpeechPriority::System
    );
}

    // Prediction + symbol grid logic
    m_aac->setPredictionEnabled(cfg.typingMethod != TypingMethod::SymbolGrid);
// ⭐ Prediction mode announcement
bool pred = (cfg.typingMethod != TypingMethod::SymbolGrid);
if (m_aac->speechEngine() && !newModes.fatigueMode) {
    m_aac->speechEngine()->speakNotification(
        pred ? tr("Prediction enabled")
             : tr("Prediction disabled"),
        SpeechPriority::System
    );
}

    // Vocabulary
    m_aac->setActiveCategory(cfg.vocabId);

    m_aac->saveProfile();
m_skipNextContextualHelp = true;
    // Onboarding complete → go to ConnectionHub
    showScreen(Screen_ConnectionHub);
// ⭐ First‑time orientation message (only if Help Mode enabled)
if (cfg.helpMode &&
    m_aac->speechEngine() &&
    !m_aac->modes().fatigueMode)
{
    m_aac->speechEngine()->speakNotification(
        tr("You are now on the Connect screen. "
           "Press F12 at any time for help."),
        SpeechPriority::System
    );
}

});
connect(m_connectionHubScreen, &ConnectionHubScreen::findServersRequested,
        this, [this]() {
    showScreen(Screen_AACServerDiscovery);
});

connect(m_connectionHubScreen, &ConnectionHubScreen::publicServersRequested,
        this, [this]() {
    showScreen(Screen_AACServerDiscovery);
});

connect(m_connectionHubScreen, &ConnectionHubScreen::recentServersRequested,
        this, [this]() {
    showScreen(Screen_AACServerDiscovery);
});

connect(m_connectionHubScreen, &ConnectionHubScreen::joinCodeRequested,
        this, [this]() {
    showScreen(Screen_AACServerDiscovery);
});

connect(m_connectionHubScreen, &ConnectionHubScreen::deepLinkRequested,
        this, [this]() {
    showScreen(Screen_AACServerDiscovery);
});

// QR scanning still bypasses discovery
connect(m_connectionHubScreen, &ConnectionHubScreen::qrScanRequested,
        this, [this]() {
    showScreen(Screen_QRScanner);
});
connect(m_connectionHubScreen, &ConnectionHubScreen::backRequested,
        this, [this]() {
    showScreen(Screen_AACSummary);
});

connect(m_AACServerDiscoveryScreen, &AACServerDiscoveryScreen::qrScanRequested,
        this, [this]() {
    showScreen(Screen_QRScanner);
});
//
// AACServerDiscoveryScreen → ConnectScreen
//
connect(m_AACServerDiscoveryScreen, &AACServerDiscoveryScreen::lanRequested,
        this, [this]() {
    showScreen(Screen_Connect);
    m_ConnectScreen->requestLanServers();
});

connect(m_AACServerDiscoveryScreen, &AACServerDiscoveryScreen::publicServersRequested,
        this, [this]() {
    showScreen(Screen_Connect);
    m_ConnectScreen->requestPublicServers();
});

connect(m_AACServerDiscoveryScreen, &AACServerDiscoveryScreen::recentServersRequested,
        this, [this]() {
    showScreen(Screen_Connect);
    m_ConnectScreen->requestRecentServers();
});

connect(m_AACServerDiscoveryScreen, &AACServerDiscoveryScreen::joinCodeRequested,
        this, [this]() {
    showScreen(Screen_Connect);
    m_ConnectScreen->requestJoinCode();
});

connect(m_AACServerDiscoveryScreen, &AACServerDiscoveryScreen::qrScanRequested,
        this, [this]() {
    showScreen(Screen_QRScanner);
});

connect(m_AACServerDiscoveryScreen, &AACServerDiscoveryScreen::deepLinkRequested,
        this, [this]() {
    showScreen(Screen_Connect);
    m_ConnectScreen->configureForDeepLinkEntry();
});

connect(m_AACServerDiscoveryScreen, &AACServerDiscoveryScreen::backRequested,
        this, [this]() {
    showScreen(Screen_ConnectionHub);
});
    //
    // CONNECT (ConnectScreen → BackendAdapter)
    //
// StateMachine → ConnectScreen reconnect UI
connect(m_stateMachine, &StateMachine::reconnecting,
        m_ConnectScreen, &ConnectScreen::showReconnectSpinner);

connect(m_stateMachine, &StateMachine::reconnectStopped,
        m_ConnectScreen, &ConnectScreen::hideReconnectSpinner);

connect(m_ConnectScreen, &ConnectScreen::connectRequested,
        this, [this](const ServerInfo& info) {

        bool ok = false;
        int portNum = info.port.toInt(&ok);
        if (!ok) {
            m_ConnectScreen->setErrorMessage(tr("Invalid port"));
            return;
        }

        m_ConnectScreen->setErrorMessage(QString());
    m_backend->connectToServer(info.host, portNum, info.username, info.password);
    });

connect(m_ConnectScreen, &ConnectScreen::reconnectCancelled,
        this, [this]() {
});
    connect(m_ConnectScreen, &ConnectScreen::manualConnectRequested,
            this, [this] {
        m_ConnectScreen->setErrorMessage(QString());
        m_ConnectScreen->configureForManual();
    });

    connect(m_ConnectScreen, &ConnectScreen::backRequested,
            this, [this] {
        m_ConnectScreen->setErrorMessage(QString());
        m_ConnectScreen->configureForMetadata(QString(), QString(), QString(), false);
const AACModeFlags modes = m_aac->modes();
if (m_aac->speechEngine() && !modes.fatigueMode)
    m_aac->speechEngine()->speakNotification(
        tr("Server metadata loaded"),
        SpeechPriority::System
    );
    });

    //
    // PUBLIC SERVERS
    //
    if (m_serverService) {
        connect(m_ConnectScreen, &ConnectScreen::publicServersRequested,
                this, [this] {
            m_ConnectScreen->setErrorMessage(QString());
            m_serverService->fetchPublicServers(true);
        });

        connect(m_serverService, &ServerService::publicServersUpdated,
                this, [this](const QList<ServerInfo>& servers) {
const AACModeFlags modes = m_aac->modes();
if (m_aac->speechEngine() && !modes.fatigueMode)
    m_aac->speechEngine()->speakNotification(
        tr("Public servers updated"),
        SpeechPriority::System
    );

            QVector<UnifiedServerEntry> entries;
            entries.reserve(servers.size());

            for (const auto& s : servers) {
                UnifiedServerEntry e;
                e.name         = s.name;
                e.host         = s.host;
                e.port         = QString::number(s.port);
                e.accountBased = false;
                entries.push_back(e);
            }

            m_ConnectScreen->setErrorMessage(QString());
            m_ConnectScreen->setServerList(entries);
        });
    }

    //
    // RECENT SERVERS
    //
    if (m_serverService) {
        connect(m_ConnectScreen, &ConnectScreen::recentServersRequested,
                this, [this] {

            m_ConnectScreen->setErrorMessage(QString());

            const QList<ServerInfo> recents = m_serverService->loadLatestHosts();

            QVector<UnifiedServerEntry> entries;
            entries.reserve(recents.size());

            for (const auto& s : recents) {
                UnifiedServerEntry e;
                e.name         = s.name;
                e.host         = s.host;
                e.port         = QString::number(s.port);
                e.accountBased = false;
                entries.push_back(e);
            }

            m_ConnectScreen->setServerList(entries);
        });
    }

    //
    // JOIN CODE
    //
    if (m_serverService) {
        connect(m_ConnectScreen, &ConnectScreen::joinCodeEntered,
                this, [this](const QString& code) {
            m_ConnectScreen->setErrorMessage(QString());
            if (!code.trimmed().isEmpty())
                m_serverService->resolveJoinCode(code);
        });

        connect(m_serverService, &ServerService::joinCodeResolved,
                this, [this](const ServerInfo& s) {
const AACModeFlags modes = m_aac->modes();
if (m_aac->speechEngine() && !modes.fatigueMode)
    m_aac->speechEngine()->speakNotification(
        tr("Join code resolved"),
        SpeechPriority::System
    );

            m_ConnectScreen->setErrorMessage(QString());
            m_ConnectScreen->configureForMetadata(
                s.name,
                s.host,
                QString::number(s.port),
                false
            );
        });

        connect(m_serverService, &ServerService::joinCodeFailed,
                this, [this](const QString& reason) {
const AACModeFlags modes = m_aac->modes();
if (m_aac->speechEngine() && !modes.fatigueMode)
    m_aac->speechEngine()->speakNotification(
        tr("Join code failed"),
        SpeechPriority::System
    );
            m_ConnectScreen->setErrorMessage(reason);
        });
    }

    //
    // LAN DISCOVERY
    //
    if (m_discovery) {
        connect(m_ConnectScreen, &ConnectScreen::lanServersRequested,
                this, [this] {
            m_ConnectScreen->setErrorMessage(QString());
            m_discovery->startDiscovery();
        });

        connect(m_discovery, &AACServerDiscovery::isScanningChanged,
                m_ConnectScreen, &ConnectScreen::onScanningChanged);

        connect(m_discovery, &AACServerDiscovery::lastUpdatedChanged,
                m_ConnectScreen, &ConnectScreen::onLastUpdatedChanged);

        connect(m_discovery, &AACServerDiscovery::serversUpdated,
                this, [this](const QVector<ServerInfo>& servers) {
const AACModeFlags modes = m_aac->modes();
if (m_aac->speechEngine() && !modes.fatigueMode)
    m_aac->speechEngine()->speakNotification(
        tr("LAN servers updated"),
        SpeechPriority::System
    );

            QVector<UnifiedServerEntry> entries;
            entries.reserve(servers.size());

            for (const auto& s : servers) {
                UnifiedServerEntry e;
                e.name         = s.name;
                e.host         = s.host;
                e.port         = QString::number(s.port);
                e.accountBased = false;
                entries.push_back(e);
            }

            m_ConnectScreen->setErrorMessage(QString());
            m_ConnectScreen->setServerList(entries);
        });
    }

//
// QR Scanner routing
//

connect(m_qrScannerScreen, &QRScannerScreen::backRequested,
        this, [this]() {
    showScreen(Screen_ConnectionHub);
});

connect(m_qrScannerScreen, &QRScannerScreen::qrPayloadDetected,
        this, [this](const QString& payload) {

    // Try deep link first
    ServerInfo info = m_serverService->resolveQRPayload(payload);

    if (info.isValid()) {
        // Deep link success → go straight to Connect
        showScreen(Screen_Connect);
        m_ConnectScreen->setMode(ConnectMode::MetadataDriven);
        m_ConnectScreen->setMetadata(info.host, info.tcpPort);
        return;
    }

    // Not a deep link → treat as join code
    showScreen(Screen_Connect);
    m_ConnectScreen->setMode(ConnectMode::MetadataDriven);

    // This will emit joinCodeResolved or joinCodeFailed
    m_serverService->resolveJoinCode(payload);
});

    //
    // CONNECTION STATE → CHANNEL LIST / CONNECT
    //
connect(m_backend, &BackendAdapter::connectionStateChanged,
        this, [this](BackendAdapter::ConnectionState st) {

    switch (st) {

    case BackendAdapter::ConnectionState::Connected:
    {
        const AACModeFlags modes = m_aac->modes();

        if (m_earcons && !modes.fatigueMode)
            m_earcons->reconnectSuccess();
    if (m_aac->speechEngine() && !modes.fatigueMode)
        m_aac->speechEngine()->speakNotification(tr("Connected"), SpeechPriority::System);

m_ConnectScreen->setErrorMessage(QString());
        m_backend->refreshChannels();
        break;
    }


    case BackendAdapter::ConnectionState::Error:
    {
        m_ConnectScreen->setErrorMessage(tr("Connection failed"));
        showScreen(Screen_Connect);
        break;
    }

    default:
        break;
    }
});

    //
    // CHANNEL LIST (server → channels → selection)
    //
    connect(m_backend, &BackendAdapter::channelsEnumerated,
            this, [this](const QList<ChannelInfo>& channels) {
        if (m_channelListScreen) {
            m_channelListScreen->setChannels(channels);
            showScreen(Screen_ChannelList);
        }
    });

    if (m_channelListScreen) {
        connect(m_channelListScreen, &ChannelListScreen::channelChosen,
                this, [this](const QString& id) {
            m_backend->joinChannel(id);
        });

        connect(m_channelListScreen, &ChannelListScreen::backRequested,
                this, [this]() {
            showScreen(Screen_Connect);
        });

        connect(m_channelListScreen, &ChannelListScreen::refreshRequested,
                this, [this]() {
            m_backend->refreshChannels();
        });
    }

    //
    // CHANNEL PASSWORD
    //
    if (m_channelPasswordScreen) {
        connect(m_channelPasswordScreen, &ChannelPasswordScreen::passwordEntered,
                this, [this](const QString& id, const QString& password) {
            m_backend->joinChannel(id, password);
        });

        connect(m_channelPasswordScreen, &ChannelPasswordScreen::backRequested,
                this, [this]() {
            showScreen(Screen_ChannelList);
        });
    }

    connect(m_backend, &BackendAdapter::channelPasswordRequired,
            this, [this](const QString& id) {
        if (!m_channelPasswordScreen)
            return;
        m_channelPasswordScreen->setChannel(id);
m_aac->speechEngine()->speakNotification(
    tr("Password required"),
    SpeechPriority::System
);
        showScreen(Screen_ChannelPassword);
    });

    //
    // CHANNEL JOIN / LEAVE → InChannelScreen
    //
    connect(m_backend, &BackendAdapter::channelJoined,
            this, [this](const QString& id) {
        Q_UNUSED(id);
if (m_earcons) m_earcons->channelJoin();
    if (m_aac->speechEngine() && !m_aac->modes().fatigueMode)
        m_aac->speechEngine()->speakNotification(
            tr("Joined channel"),
            SpeechPriority::System
        );
        showScreen(Screen_InChannel);
    });

connect(m_backend, &BackendAdapter::channelJoined,
        this, [this](const QString& id) {
    Q_UNUSED(id);

    showScreen(Screen_AACMain);

    // Auto-open AAC keyboard or symbol grid
    if (m_lastUsedAACInput == AACInput::Keyboard)
        showScreen(Screen_AACKeyboard);
    else
        showScreen(Screen_AACSymbolGrid);
});

    connect(m_inChannelScreen, &InChannelScreen::leaveChannelRequested,
            this, [this] {
if (m_earcons) m_earcons->channelLeave();
        m_backend->leaveChannel();
        m_backend->refreshChannels();
        showScreen(Screen_ChannelList);
    });

connect(m_backend, &BackendAdapter::channelForcedLeave,
        this, [this](const QString& reason) {

    const AACModeFlags modes = m_aac->modes();

    if (m_earcons && !modes.fatigueMode)
        m_earcons->channelLeave();

    if (m_aac->speechEngine() && !modes.fatigueMode)
        m_aac->speechEngine()->speakNotification(
            reason,
            SpeechPriority::System
        );

    showScreen(Screen_ChannelList);
});
    connect(m_inChannelScreen, &InChannelScreen::transmitToggled,
            this, [this](bool enabled) {
        m_backend->setTransmitEnabled(enabled);
    });

    connect(m_backend, &BackendAdapter::selfVoiceEvent,
            m_inChannelScreen, &InChannelScreen::updateSelfVoiceState);
connect(m_backend, &BackendAdapter::transmitStateChanged,
        m_inChannelScreen, &InChannelScreen::setTransmitStatus);

    connect(m_backend, &BackendAdapter::otherUserVoiceEvent,
            m_inChannelScreen, &InChannelScreen::updateOtherUserVoiceState);

connect(m_backend, &BackendAdapter::userJoinedAAC,
        this, [this](const QString& username) {
    if (m_earcons) m_earcons->userJoin();
    m_inChannelScreen->onUserJoined(username);
});

connect(m_backend, &BackendAdapter::userLeftAAC,
        this, [this](const QString& username) {
    if (m_earcons) m_earcons->userLeave();
    m_inChannelScreen->onUserLeft(username);
});
    connect(m_aacMainScreen, &AACMainScreen::clearRequested,
            this, [this]() {
        m_backend->endAutoSilenceSession();
    });

    connect(m_aacMainScreen, &AACMainScreen::doneRequested,
            this, [this]() {
        m_backend->endAutoSilenceSession();
    });

connect(m_aacMainScreen, &AACMainScreen::keyboardRequested,
        this, [this]() {
    showScreen(Screen_AACKeyboard);
});

connect(m_aacMainScreen, &AACMainScreen::symbolGridRequested,
        this, [this]() {
    showScreen(Screen_AACSymbolGrid);
});

connect(m_aacSymbolGridScreen, &AACSymbolGridScreen::keyboardRequested,
        this, [this]() {
    showScreen(Screen_AACKeyboard);
});

connect(m_aacKeyboardScreen, &AACKeyboardScreen::doneRequested,
        this, [this]() {
    showScreen(Screen_AACMain);
});

// In MainWindow::wireNavigation()
connect(m_aacKeyboardScreen, &AACKeyboardScreen::characterTyped,
        this, [this](QChar c) {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->onCharacterTyped(c);
});

connect(m_aacKeyboardScreen, &AACKeyboardScreen::backspacePressed,
        this, [this]() {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->onBackspace();
});

connect(m_aacKeyboardScreen, &AACKeyboardScreen::spacePressed,
        this, [this]() {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->onSpace();
});

connect(m_aacKeyboardScreen, &AACKeyboardScreen::enterPressed,
        this, [this]() {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->onEnterPressed();
});

connect(m_aacKeyboardScreen, &AACKeyboardScreen::clearRequested,
        this, [this]() {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->onClear();
});

connect(m_aacKeyboardScreen, &AACKeyboardScreen::deleteWordRequested,
        this, [this]() {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->onDeleteWord();
});

connect(m_aacKeyboardScreen, &AACKeyboardScreen::moveCursorLeft,
        this, [this]() {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->onMoveCursorLeft();
});

connect(m_aacKeyboardScreen, &AACKeyboardScreen::moveCursorRight,
        this, [this]() {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->onMoveCursorRight();
});

    //
    // SYMBOL GRID → AAC MAIN
    //
connect(m_aacSymbolGridScreen, &AACSymbolGridScreen::symbolActivated,
        this, [this](const QString& word) {
    if (m_earcons) m_earcons->activate();
    m_aacMainScreen->appendWord(word);
});

    //
    // SETTINGS
    //
    connect(m_settingsScreen, &AACSettingsScreen::transmitModeChanged,
            this, [this](int modeIndex) {
    if (m_earcons) m_earcons->confirm();
        auto mode = static_cast<BackendAdapter::AACTransmitMode>(modeIndex);

        m_backend->setTransmitMode(mode);
        m_inChannelScreen->setTransmitModeLabel(mode);
    });

connect(m_settingsScreen, &AACSettingsScreen::backRequested,
        this, [this] {
    if (m_earcons) m_earcons->navBack();
    showScreen(Screen_Connect);
});

connect(m_speechSettingsScreen, &AACSpeechSettingsScreen::backRequested,
        this, [this] {
    if (m_earcons) m_earcons->navBack();
    showScreen(Screen_Settings);
});
connect(m_appSettingsScreen, &AppSettingsScreen::backRequested,
        this, [this] {
    if (m_earcons) m_earcons->navBack();
    showScreen(Screen_Connect);
});

void MainWindow::showScreen(ScreenId id)
{
    QWidget* screen = m_stack->widget(static_cast<int>(id));

    int oldIndex = m_stack->currentIndex();
    int newIndex = static_cast<int>(id);

    if (oldIndex != -1 &&
        oldIndex != newIndex &&        // screen actually changed
m_earcons) {
        if (newIndex > oldIndex)
            m_earcons->navForward();
        else if (newIndex < oldIndex)
            m_earcons->navBack();
    }

    m_stack->setCurrentWidget(screen);

    if (auto* base = qobject_cast<AACScreenBase*>(screen)) {
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

    if (auto* aacScreen = currentAACScreen()) {
if (m_skipNextContextualHelp) {
    m_skipNextContextualHelp = false;
    return;
}
        m_aac->setActiveScreen(aacScreen);
        if (m_earcons)
            m_earcons->focusInit();

AACModeFlags modes = m_aac->modes();
if (modes.helpMode) {
    speakContextualHelp();
}
    }
}

AACScreenAdapter* MainWindow::currentAACScreen() const
{
    return qobject_cast<AACScreenAdapter*>(m_stack->currentWidget());
}
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::FocusIn) {
        AACModeFlags modes = m_aac->modes();
        if (modes.helpMode && !m_skipNextContextualHelp) {
            speakContextualHelp();
        }
    }

    return QMainWindow::eventFilter(obj, event);
}
bool MainWindow::isCommunicationContext() const {
    ScreenId id = static_cast<ScreenId>(m_stack->currentIndex());
    return (id == Screen_InChannel ||
            id == Screen_AACMain ||
            id == Screen_AACKeyboard ||
            id == Screen_AACSymbolGrid);
}
void MainWindow::closeEvent(QCloseEvent* event)
{
    m_backend->shutdown();
    QMainWindow::closeEvent(event);
}
void MainWindow::keyPressEvent(QKeyEvent* e)
{
    const AACModeFlags modes = m_aac->modes();

    //
    // Helper: Navigation speech (suppressed in fatigue mode)
    //
    auto speakNav = [&](const QString& text) {
        if (m_aac && m_aac->speechEngine() && !modes.fatigueMode) {
            // Suppress prediction speech during navigation
            m_aac->predictionEngine()->suspendForNavigation();

            m_aac->speechEngine()->speakNotification(
                text,
                SpeechPriority::Navigation
            );
        }
    };

    //
    // Helper: Navigation haptic
    //
    auto hapticNav = [&]() {
        if (m_aac && m_aac->feedbackEngine())
            m_aac->feedbackEngine()->hapticSoft();
    };

    //
    // Helper: Activation haptic (F6/F7 actions)
    //
    auto hapticActivate = [&]() {
        if (m_aac && m_aac->feedbackEngine())
            m_aac->feedbackEngine()->hapticMedium();
    };

    //
    // Helper: Error haptic (unused here but included for completeness)
    //
    auto hapticError = [&]() {
        if (m_aac && m_aac->feedbackEngine())
            m_aac->feedbackEngine()->hapticStrong();
    };

    //
    // ────────────────────────────────────────────────
    //  UNIVERSAL AAC NAVIGATION
    // ────────────────────────────────────────────────
    //
    if (e->key() == Qt::Key_Escape) {
        if (m_titleBar)
            m_titleBar->triggerBack();
        if (m_earcons) m_earcons->navBack();
        hapticNav();
        speakNav(tr("Back"));
        return;
    }

    //
    // ────────────────────────────────────────────────
    //  CONTEXTUAL HELP (Shift + F12)
    // ────────────────────────────────────────────────
    //
    if (e->key() == Qt::Key_F12 && (e->modifiers() & Qt::ShiftModifier)) {
        if (m_earcons) m_earcons->navForward();
        hapticNav();
        speakNav(tr("Help"));
        speakContextualHelp();
        return;
    }

    //
    // ────────────────────────────────────────────────
    //  AAC SETTINGS CLUSTER (F1–F3)
    // ────────────────────────────────────────────────
    //
    if (e->key() == Qt::Key_F1) {
        if (m_earcons) m_earcons->navForward();
        hapticNav();
        showScreen(Screen_Settings);
        speakNav(tr("AAC settings"));
        return;
    }

    if (e->key() == Qt::Key_F2) {
        if (m_earcons) m_earcons->navForward();
        hapticNav();
        showScreen(Screen_SpeechSettings);
        speakNav(tr("Speech settings"));
        return;
    }

    if (e->key() == Qt::Key_F3) {
        if (m_earcons) m_earcons->navForward();
        hapticNav();
        showScreen(Screen_AppSettings);
        speakNav(tr("App settings"));
        return;
    }

    //
    // ────────────────────────────────────────────────
    //  AAC COMMUNICATION CLUSTER (F4–F7)
    // ────────────────────────────────────────────────
    //
    if (e->key() == Qt::Key_F4) {
        if (isCommunicationContext()) {
            if (m_earcons) m_earcons->navForward();
            hapticNav();

            m_lastUsedAACInput = AACInput::Keyboard;
            showScreen(Screen_AACKeyboard);

            // Announce current AAC input mode
            speakNav(tr("Keyboard"));
        }
        return;
    }

    if (e->key() == Qt::Key_F5) {
        if (isCommunicationContext()) {
            if (m_earcons) m_earcons->navForward();
            hapticNav();

            m_lastUsedAACInput = AACInput::SymbolGrid;
            showScreen(Screen_AACSymbolGrid);

            // Announce current AAC input mode
            speakNav(tr("Symbol grid"));
        }
        return;
    }

    if (e->key() == Qt::Key_F6) {
        if (isCommunicationContext() && m_aacMainScreen) {
            if (m_earcons) m_earcons->navForward();
            hapticActivate();
            m_aacMainScreen->commitText();
            speakNav(tr("Speak message"));
        }
        return;
    }

    if (e->key() == Qt::Key_F7) {
        if (isCommunicationContext() && m_aacMainScreen) {
            if (m_earcons) m_earcons->navForward();
            hapticActivate();
            m_aacMainScreen->clearText();
            speakNav(tr("Clear message"));
        }
        return;
    }

    //
    // ────────────────────────────────────────────────
    //  SERVER DISCOVERY (F12)
    // ────────────────────────────────────────────────
    //
    if (e->key() == Qt::Key_F12) {
        if (m_earcons) m_earcons->navForward();
        hapticNav();
        showScreen(Screen_AACServerDiscovery);
        speakNav(tr("Server discovery"));
        return;
    }

    //
    // ────────────────────────────────────────────────
    //  FALLBACK TO BASE CLASS
    // ────────────────────────────────────────────────
    //
    QMainWindow::keyPressEvent(e);
}
void MainWindow::speakContextualHelp()
{
    if (!m_aac || !m_aac->speechEngine())
        return;

    AACModeFlags modes = m_aac->modes();

    // Optional: help earcon
    if (m_earcons)
        m_earcons->help();

    // Optional: help haptic
    if (m_aac->feedbackEngine())
        m_aac->feedbackEngine()->hapticSoft();

    auto* screen = currentAACScreen();
    if (!screen) {
        m_aac->speechEngine()->speakNotification(
            tr("Help not available for this screen."),
            SpeechPriority::Navigation
        );
        return;
    }

    ScreenId id = static_cast<ScreenId>(m_stack->currentIndex());

    QString focusId;
    if (QWidget* f = focusWidget())
        focusId = f->objectName();

    QString text;

    // 1. Screen override: element-level
    text = screen->contextualHelpForElement(focusId);

    // 2. Registry: element-level
    if (text.isEmpty())
        text = m_helpRegistry[id].elementHelp.value(focusId);

    // 3. Screen override: screen-level
    if (text.isEmpty())
        text = screen->screenLevelHelp();

    // 4. Registry: screen-level
    if (text.isEmpty())
        text = m_helpRegistry[id].screenHelp;

    // 5. Fallback
    if (text.isEmpty())
        text = tr("Help not available for this screen.");

    // Suppress prediction speech
    m_aac->predictionEngine()->suspendForNavigation();

    // Fatigue-aware speech
    if (!modes.fatigueMode) {
        m_aac->speechEngine()->speakNotification(
            text,
            SpeechPriority::Navigation
        );
    }
}
void MainWindow::onSendToChannel(const AACMessage& msg)
{
    if (!m_backend)
        return;

    AACMessage enriched = msg;

    m_backend->sendChannelMessage(enriched);

    if (m_aac && m_aac->history())
        m_aac->history()->addMessage(enriched.text);
}
void MainWindow::onSendToUser(const AACMessage& msg)
{
    if (!m_backend)
        return;

    AACMessage enriched = msg;

    m_backend->sendPrivateMessage(enriched);

    if (m_aac && m_aac->history())
        m_aac->history()->addMessage(enriched.text);
}
void MainWindow::onSpeakMessage(const AACMessage& msg)
{
    if (!m_aac || !m_aac->speechEngine())
        return;

    m_aac->speechEngine()->speakNotification(
        msg.text,
        SpeechPriority::User
    );

    if (m_aac->history())
        m_aac->history()->addMessage(msg.text);
}
