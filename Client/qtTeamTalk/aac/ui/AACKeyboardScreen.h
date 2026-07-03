#pragma once

#include <QWidget>
#include <QVector>
#include <QStringList>
#include <QPointer>

class AACAccessibilityManager;
class AACInputController;
class AACKeyButton;
class PredictiveStrip;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTimer;
class QKeyEvent;

class AACKeyboardScreen : public QWidget
{
    Q_OBJECT

public:
    enum KeyboardMode {
        LettersMode,
        NumbersMode,
        SymbolsMode,
        GridMode
    };
    Q_ENUM(KeyboardMode)

    explicit AACKeyboardScreen(AACAccessibilityManager* accessibility,
                               QWidget* parent = nullptr);
    ~AACKeyboardScreen();

    void setText(const QString& text);
    void updateCursorContext(int cursorPosition, const QString& text);
    void setPredictions(const QStringList& words);

    enum CursorPlacementMode {
        CursorAfterSpace,         // Proloquo style
        CursorAfterWord,          // LAMP style
        CursorBetweenWordAndSpace, // TD Snap style
    CursorAfterPunctuation       // ⭐ NEW
    };

public slots:
    void setMode(KeyboardMode mode);
    void onFreezeStateChanged(bool frozen);
    void onHighContrastChanged(bool enabled);

    // Curated strip scanning toggle
    void setCuratedStripScanningEnabled(bool enabled) {
        m_scanCuratedStrip = enabled;
        updateUnifiedHighlight();
    }

    // Scanning entry points
    void startRowScan();
    void startColumnScan();
    void activateScanTarget();

    // Step scanning controls
    void handleStepNext();
    void handleStepPrevious();
    void handleStepSelect();

    // Dwell tick (for curated strip dwell)
    void onDwellTick();

    // High‑level toggles (for external controller)
    void setCoreSymbolsFirst(bool enabled);
    void setCuratedStripDwellEnabled(bool enabled);
    void setHighContrastEnabled(bool enabled);
    void setFreezeEnabled(bool enabled);
    void stopScan();
    void setSemanticHighlight(const QString& tag);

signals:
    void characterTyped(const QString& text);
    void backspacePressed();
    void spacePressed();
    void enterPressed();
    void actionTriggered(const QString& tag);
    void symbolSemantic(const QString& tag);
    void modeChanged(KeyboardMode mode);
    void shiftStateChanged(bool shiftOn);
    void predictionInserted(const QString& word);
    void replaceText(const QString& text, int cursorPosition);

    // Curated strip semantic context
    void curatedStripSymbolsChanged(const QStringList& symbols);

    // Grid navigation
    void moveCursorLeft();
    void moveCursorRight();
    void clearRequested();
    void deleteWordRequested();

    void doneRequested();

protected:
    // UI builders
    void buildUi();
    void buildTopRow();
    void buildCuratedSymbolStrip();
    void buildKeyboardArea();
    void buildControlRow();

    // Layout builders
    void buildLettersLayout();
    void buildNumbersLayout();
    void buildSymbolsLayout();
    void buildGridLayout();

    // Keyboard content population
    void populateLettersRows();
    void populateNumbersRows();
    void populateSymbolsRows();
    void populateGridItems();

    // Popup
    QWidget* buildPopupForKey(AACKeyButton* btn);

    // Key handling
    void handleKeyButtonActivated(const QString& text);
    void handleBackspaceActivated();
    void handleEnterActivated();
    void handleSpaceActivated();

    // Auto-capitalization + spacing
    bool shouldAutoCapitalize(const QString& text, int cursorPos) const;
    QString applyAutoCapitalization(const QString& input) const;
    QString applySmartSpacing(const QString& typed) const;

    // Shift / CapsLock
    void toggleShift();
    void toggleCapsLock();

    // Highlight helpers
    QString currentTokenAtCursor() const;
    void replaceTokenAtCursor(const QString& replacement);
    void predictionChosen(const QString& word);

    // Visual settings
    void applyVisualSettings();

    // Keyboard navigation via arrow keys
    void keyPressEvent(QKeyEvent* e) override;

private:
    // Core managers
    AACAccessibilityManager* m_accessibility = nullptr;
    AACInputController* m_inputController = nullptr;

    // Layouts
    QVBoxLayout* m_mainLayout = nullptr;
    QHBoxLayout* m_topRowLayout = nullptr;
    QHBoxLayout* m_curatedStripLayout = nullptr;
    QVBoxLayout* m_keyboardLayout = nullptr;
    QHBoxLayout* m_controlRowLayout = nullptr;
    QHBoxLayout* m_predictiveLayout = nullptr;

    QWidget* m_topRowWidget = nullptr;
    QWidget* m_curatedStripWidget = nullptr;
    QWidget* m_keyboardWidget = nullptr;
    QWidget* m_controlRowWidget = nullptr;
    PredictiveStrip* m_predictiveStrip = nullptr;

    QGridLayout* m_keyboardGrid = nullptr;

    // Buttons
    AACKeyButton* m_lettersModeButton = nullptr;
    AACKeyButton* m_numbersModeButton = nullptr;
    AACKeyButton* m_symbolsModeButton = nullptr;
    AACKeyButton* m_gridModeButton = nullptr;

    AACKeyButton* m_backspaceButton = nullptr;
    AACKeyButton* m_spaceButton = nullptr;
    AACKeyButton* m_enterButton = nullptr;

    AACKeyButton* m_shiftButtonLeft = nullptr;
    AACKeyButton* m_shiftButtonRight = nullptr;

    // State
    CursorPlacementMode m_cursorPlacement = CursorAfterSpace;
    QString m_currentText;
    int m_cursorPosition = 0;

    bool m_shift = false;
    bool m_capsLock = false;
    bool m_frozen = false;
    bool m_highContrast = false;

    // Curated strip scanning toggle
    bool m_scanCuratedStrip = true;

    // Scanning
    bool m_scanning = false;
    int m_scanRow = 0;
    int m_scanCol = 0;
    QPointer<AACKeyButton> m_currentHighlightedButton;

    // Scanning behaviour flags
    bool m_coreSymbolsFirst = true;   // curated strip row first when scanning
    bool m_curatedStripDwell = false; // dwell activation on curated strip
QTimer* m_scanTimer = nullptr;

    // Backspace repeat
    QTimer* m_backspaceRepeatTimer = nullptr;

    // Keyboard mode
    KeyboardMode m_mode = LettersMode;

    // Rows
    QVector<QStringList> m_lettersRows;
    QVector<QStringList> m_numbersRows;
    QVector<QStringList> m_symbolsRows;
    QStringList m_gridItems;

    // Highlight helpers
    AACKeyButton* highlightedButton() const;
    void updateUnifiedHighlight();

    void moveHighlightLeft();
    void moveHighlightRight();
    void moveHighlightUp();
    void moveHighlightDown();

    int maxRow() const;
    int maxCol() const;

    void moveHighlightToNextItem();
    void moveHighlightToPreviousItem();
};
