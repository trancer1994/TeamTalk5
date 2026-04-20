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

class UnifiedConnectScreen;
class InChannelScreen;
class AACMainScreen;
class AACKeyboardScreen;
class AACSymbolGridScreen;
class AACSettingsScreen;
class AACSpeechSettingsScreen;
class AppSettingsScreen;

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
        Screen_UnifiedConnect = 0,
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
    // Core services
    AACAccessibilityManager* m_aac = nullptr;
    BackendAdapter*          m_backend = nullptr;
    AACServerDiscovery*      m_discovery = nullptr;
    ServerService*           m_serverService = nullptr;

    // Window structure
    QWidget*      m_central = nullptr;
    QVBoxLayout*  m_layout = nullptr;
    AACTitleBar*  m_titleBar = nullptr;

    // Screen stack
    QStackedWidget* m_stack = nullptr;

    // Screens
    UnifiedConnectScreen*    m_unifiedConnectScreen = nullptr;
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
};
