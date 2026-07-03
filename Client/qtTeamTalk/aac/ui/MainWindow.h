#pragma once

#include <QMainWindow>
#include <QStackedWidget>

class AACTitleBar;
class AACScreenBase;
class AACScreenAdapter;
class AACAccessibilityManager;
class BackendAdapter;
class AACServerDiscovery;
class ServerService;

class AACCommunicationMethodScreen;
class AACTypingMethodScreen;
class AACVocabularyScreen;
class AACSetupSummaryScreen;
class ConnectionHubScreen; // forward declare
class AACServerDiscoveryScreen;
class ConnectScreen;
class ChannelListScreen;
class ChannelPasswordScreen;
class InChannelScreen;
class AACMainScreen;
class AACKeyboardScreen;
class AACSymbolGridScreen;
class AACSettingsScreen;
class AACSpeechSettingsScreen;
class AppSettingsScreen;


struct AACMessage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(AACAccessibilityManager* aac,
                        BackendAdapter* backend,
                        AACServerDiscovery* discovery,
                        ServerService* serverService,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

    enum ScreenId {
Screen_AACCommMethod,
Screen_AACTypingMethod,
Screen_AACVocabulary,
Screen_AACSummary,
Screen_ConnectionHub,
Screen_AACServerDiscovery,
        Screen_Connect,
        Screen_InChannel,
        Screen_AACMain,
        Screen_AACKeyboard,
        Screen_AACSymbolGrid,
        Screen_Settings,
        Screen_SpeechSettings,
        Screen_AppSettings
    };

    void showScreen(ScreenId id);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
void onSendToChannel(const AACMessage& msg);
void onSendToUser(const AACMessage& msg);
void onSpeakMessage(const AACMessage& msg);

    // Core services
    AACAccessibilityManager* m_aac = nullptr;
    BackendAdapter*          m_backend = nullptr;
    AACServerDiscovery*      m_discovery = nullptr;
    ServerService*           m_serverService = nullptr;
AACEarconRouter* m_earcons = nullptr;

    // Window structure
    QWidget*      m_central = nullptr;
    QVBoxLayout*  m_layout = nullptr;
    AACTitleBar*  m_titleBar = nullptr;

    // Screen stack
    QStackedWidget* m_stack = nullptr;

    // Screens
AACCommunicationMethodScreen* m_aacCommMethodScreen = nullptr;
AACTypingMethodScreen*        m_aacTypingMethodScreen = nullptr;
AACVocabularyScreen*          m_aacVocabularyScreen = nullptr;
AACSetupSummaryScreen*        m_aacSummaryScreen = nullptr;
ConnectionHubScreen* m_connectionHubScreen = nullptr;
    ConnectScreen*    m_ConnectScreen = nullptr;
ChannelListScreen* m_channelListScreen = nullptr;
ChannelPasswordScreen* m_channelPasswordScreen = nullptr;
    InChannelScreen*         m_inChannelScreen = nullptr;
    AACMainScreen*           m_aacMainScreen = nullptr;
    AACKeyboardScreen*       m_aacKeyboardScreen = nullptr;
    AACSymbolGridScreen*     m_aacSymbolGridScreen = nullptr;
    AACSettingsScreen*       m_settingsScreen = nullptr;
    AACSpeechSettingsScreen* m_speechSettingsScreen = nullptr;
    AppSettingsScreen*       m_appSettingsScreen = nullptr;

    // Internal helpers
    void createScreens();
    void wireNavigation();
    void applyInitialFocus(ScreenId id);
    AACScreenAdapter* currentAACScreen() const;
bool isCommunicationContext() const;
enum class AACInput { Keyboard, SymbolGrid };
AACInput m_lastUsedAACInput = AACInput::Keyboard;

void initHelpRegistry();
void speakContextualHelp();
bool m_skipNextContextualHelp = false;

struct HelpEntry {
    QString screenHelp;
    QHash<QString, QString> elementHelp; // key = element ID
};

QHash<ScreenId, HelpEntry> m_helpRegistry;
};
