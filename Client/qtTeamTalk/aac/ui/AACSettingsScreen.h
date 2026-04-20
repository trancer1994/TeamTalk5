#pragma once

#include "AACScreenBase.h"
#include "AACModeFlags.h"

class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QSlider;
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

    // Core access toggles
    AACToggle* m_largeTargets   = nullptr;
    AACToggle* m_dwell          = nullptr;
    AACToggle* m_scanning       = nullptr;
    AACToggle* m_auditory       = nullptr;
    AACToggle* m_haptics        = nullptr;
    AACToggle* m_ultraMinimal   = nullptr;
    AACToggle* m_oneHand        = nullptr;
    AACToggle* m_predictive     = nullptr;

    // Extra layout / behaviour
    QComboBox* m_gridSize       = nullptr;  // Large / Medium / Dense
    QComboBox* m_highlightStyle = nullptr;  // Ring / Glow / High contrast / Invert
    QSlider*   m_scanningSpeed  = nullptr;  // Only meaningful if scanning on
    QSlider*   m_touchHoldDelay = nullptr;  // Touch hold delay

    // One‑hand side selector
    QComboBox* m_oneHandSide    = nullptr;

    // Buttons
    AACButton* m_resetButton          = nullptr;
    AACButton* m_presetTouch          = nullptr;
    AACButton* m_presetEyeGaze        = nullptr;
    AACButton* m_presetSwitch         = nullptr;
    AACButton* m_presetCognitiveLow   = nullptr;
    AACButton* m_presetRecommended    = nullptr;
    AACButton* m_speechSettingsButton = nullptr;
    AACButton* m_backButton           = nullptr;

    QVector<QWidget*> m_interactive;
};
