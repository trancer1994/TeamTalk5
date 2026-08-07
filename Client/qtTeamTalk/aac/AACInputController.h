#pragma once

#include <QObject>
#include <QVector>
#include <QPointer>
#include <QHash>
#include <QTimer>

class AACAccessibilityManager;
class AACKeyButton;
class AACSLPOverlay;
class QWidget;
class QGridLayout;
class QHBoxLayout;

// AAC‑idiomatic node abstraction (Qt‑free inside core)
class AACNode {
public:
    QString id;      // optional identifier
    QString label;   // display text / symbol
};

struct AACGrid {
    QVector<QVector<AACNode*>> rows;
};

struct AACStrip {
    QVector<AACNode*> nodes;
};

class AACInputController : public QObject
{
    Q_OBJECT

public:
    explicit AACInputController(AACAccessibilityManager* accessibility,
                                QObject* parent = nullptr);

    // Accessor for embedded AAC core (for SLP overlay, analytics, etc.)
    Core& core() { return m_core; }
    const Core& core() const { return m_core; }
    //
    // Input intent (modality‑agnostic)
    //
    enum class InputIntent {
        MoveNext,
        MovePrev,
        MoveUp,
        MoveDown,
        Activate,
        Cancel,
        JumpToGroup,
        Emergency
    };

    void handleIntent(InputIntent intent);

    //
    // Layout ingestion (Qt → AAC core)
    //
    void setKeyboardLayout(QGridLayout* grid,
                           QHBoxLayout* curatedStrip = nullptr,
                           QHBoxLayout* controlRow   = nullptr);

    void handleHoverKey(AACKeyButton* btn);
    void handleActivation(AACKeyButton* btn);

    //
    // Scanning configuration (public API)
    //
    enum ScanMode {
        ScanOff,
        ScanLinear,
        ScanRowColumn,
        ScanByGroup,
        ScanLoop,
        ScanBounce
    };

    void setScanMode(ScanMode mode);
    void setDwellEnabled(bool enabled);
    void setDwellTime(int ms);
    void setStepScanEnabled(bool enabled);
    void setSwitchControlEnabled(bool enabled);

signals:
    void highlightChanged(QWidget* current);
    void activationRequested(AACKeyButton* btn);
    void semanticHighlightChanged(const QString& tag);

    void rowEntered(int rowIndex);
    void groupEntered(const QString& groupName);
    void itemEntered(AACKeyButton* btn);
    void itemActivated(AACKeyButton* btn);

private:
    AACAccessibilityManager* m_accessibility = nullptr;

    // Qt timers wrap core clock
    QTimer m_dwellTimer;
    QTimer m_scanStepTimer;

    //
    // Embedded AAC core (Qt‑free)
    //
    struct Core {
        //
        // AAC types
        //
        enum class ControllerState {
            Idle,
            Navigation,
            Scanning,
            Dwell,
            StepScan,
            SemanticScan,
            MotorPlan,
            Emergency,
            Simulation
        };

        enum class ScanMode {
            ScanOff,
            ScanLinear,
            ScanRowColumn,
            ScanByGroup,
            ScanLoop,
            ScanBounce
        };

        enum class SemanticGroup {
            Emotion,
            Need,
            Social,
            Question,
            Control,
            Letter,
            Number,
            Symbol
        };

        //
        // 1. Device‑agnostic access
        //
        enum class DeviceType {
            Unknown,
            Switch,
            EyeTracker,
            HeadTracker,
            Touch,
            Gesture,
            Gamepad,
            Joystick,
            SipAndPuff,
            ExternalAT
        };

        struct DeviceConfig {
            DeviceType type = DeviceType::Unknown;
            QString id;              // device identifier
            QHash<QString, QVariant> params; // thresholds, sensitivities, etc.
        };

        QVector<DeviceConfig> devices;

        void registerDevice(const DeviceConfig& cfg);
        void updateDeviceConfig(const QString& id, const QHash<QString, QVariant>& params);
        DeviceConfig deviceById(const QString& id) const;

        //
        // 2. Layout compiler (AACGrid/AACStrip/JSON → layout)
        //
        void compileLayoutFromGrid(const AACGrid& grid);
        void compileLayoutFromStrip(const AACStrip& strip);
        void compileLayoutFromJson(const QString& jsonText);

        //
        // 3. Semantic compiler
        //
        void compileSemanticsFromJson(const QString& jsonText);
        void compileSemanticsFromTags(const QHash<QString, QString>& tagToGroup);

        //
        // 4. Motor‑plan authoring
        //
        void defineMotorPlan(SemanticGroup group,
                             const QVector<QPair<int,int>>& path);
        void clearMotorPlan(SemanticGroup group);

        //
        // 5. Semantic prediction (richer)
        //
        SemanticGroup predictNextGroup() const;
        AACNode*      predictNextNode() const;

        //
        // 6. Adaptive scanning profiles
        //
        struct ScanProfile {
            int baseDwellMs      = 800;
            int minDwellMs       = 400;
            int maxDwellMs       = 1500;
            int scanStepMs       = 800;
            int adaptEveryNActs  = 10;
        };

        ScanProfile scanProfile;

        void applyScanProfile();
        void adaptScanProfile();

        //
        // 7. Evolving accessibility profiles
        //
        struct ProfileHistoryEntry {
            Profile profile;
            int totalActivations = 0;
            QDateTime timestamp;
        };

        QVector<ProfileHistoryEntry> profileHistory;

        void recordProfileUsage();
        void evolveProfile();

        //
        // 8. Safety layers
        //
        struct SafetyConfig {
            bool requireConfirmForEmergency = true;
            bool requireConfirmForDestructive = true;
            bool autoUndoAccidental = true;
            int  accidentalThresholdMs = 500;
        };

        SafetyConfig safety;

        void applySafetyLayers();
        bool shouldConfirmEmergency() const;
        bool shouldConfirmDestructive() const;

        //
        // 9. Session analytics
        //
        struct SessionAnalytics {
            int totalIntents        = 0;
            int totalErrors         = 0;
            int totalCorrections    = 0;
            int totalEmergencyJumps = 0;
            int totalMotorPlanExecs = 0;

            QHash<ControllerState, int> timeInState; // rough counts
        };

        SessionAnalytics analytics;

        void beginSession();
        void endSession();
        void recordStateSample();
        void recordError();
        void recordCorrection();

        //
        // 10. Remote control / SLP mode
        //
        bool slpRemoteActive = false;

        void setSLPRemoteActive(bool active);
        void remoteSetHighlight(int r, int c);
        void remoteActivateCurrent();
        void remoteJumpToGroup(SemanticGroup group);

        //
        // 11. Multi‑user profiles
        //
        struct UserProfileData {
            QString userId;
            Profile profile;
            ScanProfile scanProfile;
            SafetyConfig safety;
            UsageStats usage;
        };

        QHash<QString, UserProfileData> userProfiles;
        QString currentUserId;

        void setCurrentUser(const QString& userId);
        void loadUserProfile(const QString& userId);
        void saveUserProfile(const QString& userId);

        //
        // 12. Export/import of everything
        //
        QString exportStateToJson() const;
        void    importStateFromJson(const QString& jsonText);

        //
        // 13. Plug‑in architecture
        //
        struct Plugin {
            QString id;
            std::function<void(Core&)> onLoad;
            std::function<void(Core&)> onUnload;
        };

        QHash<QString, Plugin> plugins;

        void registerPlugin(const Plugin& plugin);
        void unloadPlugin(const QString& id);

        //
        // 14. Cross‑platform rendering hooks (UI‑agnostic)
        //
        std::function<void(const QVector<QVector<AACNode*>>&)> onLayoutChanged;
        std::function<void(const UsageStats&)>                  onUsageUpdated;
        std::function<void(const SessionAnalytics&)>            onAnalyticsUpdated;

        void notifyLayoutChanged();
        void notifyUsageUpdated();
        void notifyAnalyticsUpdated();

        //
        // 15. Formal testing harness
        //
        struct TestCase {
            QString id;
            QVector<InputIntent> sequence;
        };

        QVector<TestCase> testCases;

        void addTestCase(const TestCase& tc);
        void runTestCase(const QString& id);
        void runAllTestCases();

        struct SemanticDescriptor {
            SemanticGroup group;
            QString tag;
            QString phrase;
            int urgency    = 0;
            int politeness = 0;
            int intensity  = 0;
        };

        enum class Profile {
            Default,
            Blind,
            LowVision,
            MotorImpaired,
            CognitiveImpaired,
            SwitchUser,
            EyeTrackingUser
        };

        //
        // Layout
        //
        QVector<QVector<AACNode*>> layout;
        QVector<AACNode*> curatedStrip;
        QVector<AACNode*> controlRow;

        int row = 0;
        int col = 0;

        AACNode* prevHighlighted = nullptr;

        QHash<AACNode*, SemanticDescriptor> semantics;

        struct UsageStats {
            QHash<QString, int> activationsByTag;
            QHash<SemanticGroup, int> activationsByGroup;
            int totalActivations = 0;
        } usage;

        ScanMode scanMode = ScanMode::ScanOff;
        bool dwellEnabled = false;
        int  dwellTimeMs  = 800;
        bool stepScanEnabled = false;
        bool switchControlEnabled = false;

        ControllerState state = ControllerState::Idle;
        Profile profile = Profile::Default;

        int eyeAccumX = 0;
        int eyeAccumY = 0;
        int headAccumX = 0;
        int headAccumY = 0;

        QHash<SemanticGroup, QVector<QPair<int,int>>> motorPlans;

        bool simulationActive = false;
        struct IntentEvent {
            InputIntent intent;
        };
        QVector<IntentEvent> eventLog;

        //
        // Core callbacks (wired to Qt in outer class)
        //
        std::function<void(AACNode*)> onHighlightChanged;
        std::function<void(AACNode*)> onActivationRequested;
        std::function<void(const QString&)> onSemanticHighlightChanged;
        std::function<void(int)> onRowEntered;
        std::function<void(SemanticGroup)> onGroupEntered;
        std::function<void(AACNode*)> onItemEntered;
        std::function<void(AACNode*)> onItemActivated;

        //
        // Core methods
        //
        void setState(ControllerState s);
        void clampPosition();
        void updateHighlight();
        void moveLeft();
        void moveRight();
        void moveUp();
        void moveDown();
        void activateCurrent();
        void handleActivation(AACNode* node);
        void handleHoverNode(AACNode* node);
        void updateSemanticHighlight(AACNode* node);

        void setScanMode(ScanMode mode);
        void setDwellEnabled(bool enabled);
        void setDwellTime(int ms);
        void setStepScanEnabled(bool enabled);
        void setSwitchControlEnabled(bool enabled);

        void advanceScanPosition();

        SemanticGroup mostUsedGroup() const;
        AACNode* mostUrgentItem() const;
        void jumpToSemanticGroup(SemanticGroup group);
        void jumpToMostUsedGroup();
        void jumpToEmergencyItem();

        void setProfile(Profile profile);
        void applyProfileSettings();
        void adaptScanningAndDwell();
        void applySafetyModes();

        void handleSwitchInput(bool pressed);
        void handleEyeTrackingMove(int dx, int dy);
        void handleHeadTrackingMove(int dx, int dy);
        void handleGesture(const QString& gestureId);

        void learnMotorPlanForGroup(SemanticGroup group);
        void executeMotorPlan(SemanticGroup group);

        void startSimulation();
        void stopSimulation();
        void replayLastSession();
        void logIntent(InputIntent intent);
        void replayEvents(const QVector<IntentEvent>& events);

        InputIntent predictNextIntent() const;
        InputIntent correctIntentIfNeeded(InputIntent intent) const;
    } m_core;

    //
    // Qt ↔ AAC mapping
    //
    AACNode* widgetToNode(QWidget* w);
    QWidget* nodeToWidget(AACNode* n);

    void rebuildCoreLayout(QGridLayout* grid,
                           QHBoxLayout* curatedStrip,
                           QHBoxLayout* controlRow);

private slots:
    void onDwellTimeout();
    void onScanStepTimeout();
};
