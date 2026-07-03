#pragma once

#include "AACScreenBase.h"
#include "AACModeFlags.h"

class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QSlider;
class QLabel;
class AACAccessibilityManager;
class AACToggle;
class AACButton;

class AACSettingsScreen : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACSettingsScreen(AACAccessibilityManager* aac,
                               QWidget* parent = nullptr);

signals:
    void transmitModeChanged(int modeIndex);
    void backRequested();
    void speechSettingsRequested();

private slots:
    void onModesChanged(const AACModeFlags& modes);
    void applyToManager();

    void applyRecommendedDefaults();
    void applyPresetTouch();
    void applyPresetEyeGaze();
    void applyPresetSwitch();
    void applyPresetCognitiveLow();

    void onGridSizeChanged(int index);
    void onHighlightStyleChanged(int index);
    void onScanningSpeedChanged(int value);
    void onTouchHoldDelayChanged(int value);

private:
    void buildUI();
    void syncFromManager();
    void applyPreset(const AACModeFlags& modes);

    // AACScreenAdapter overrides
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;

private:
    AACAccessibilityManager* m_aac = nullptr;

    QVBoxLayout* m_rootLayout = nullptr;

    //
    // ACCESS
    //
    AACToggle* m_largeTargets   = nullptr;
    AACToggle* m_dwell          = nullptr;
    AACToggle* m_scanning       = nullptr;
    AACToggle* m_oneHand        = nullptr;
    AACToggle* m_ultraMinimal   = nullptr;

    // Access timing controls (indented)
    QSlider*   m_dwellTime      = nullptr;
    QSlider*   m_scanningSpeed  = nullptr;
    QSlider*   m_touchHoldDelay = nullptr;

    // One‑hand side selector
    QComboBox* m_oneHandSide    = nullptr;

    //
    // FEEDBACK
    //
    AACToggle* m_auditory       = nullptr;
    AACToggle* m_haptics        = nullptr;
    QComboBox* m_highlightStyle = nullptr;

    //
    // PREDICTION
    //
    AACToggle* m_predictive        = nullptr;
    AACToggle* m_coreSymbolsFirst  = nullptr;
    AACToggle* m_curatedStripDwell = nullptr;

    //
    // LAYOUT
    //
    QComboBox* m_gridSize       = nullptr;

    //
    // Communication
    //

AACToggle* m_autoReconnect = nullptr;
AACToggle* m_speakIncoming = nullptr;
QComboBox* m_transmitMode = nullptr;
    //
    // ACCESS TEST AREA
    //
    QVector<AACButton*> m_testButtons;

    //
    // PRESETS
    //
    AACButton* m_presetTouch          = nullptr;
    AACButton* m_presetEyeGaze        = nullptr;
    AACButton* m_presetSwitch         = nullptr;
    AACButton* m_presetCognitiveLow   = nullptr;
    AACButton* m_presetRecommended    = nullptr;

    //
    // NAVIGATION
    //
    AACButton* m_resetButton          = nullptr;
    AACButton* m_speechSettingsButton = nullptr;
    AACButton* m_backButton           = nullptr;

    //
    // INTERACTIVE LIST
    //
    QVector<QWidget*> m_interactive;
};
