#pragma once

#include <QWidget>
#include <QPointer>
#include <QStringList>

class AACFramework;
class AACAccessibilityManager;
class AACInputController;
class AACKeyButton;
class QGridLayout;
class QLabel;

class AACKeyboardScreen : public QWidget
{
    Q_OBJECT

public:
    enum KeyboardMode {
        LettersMode,
        NumbersMode,
        SymbolsMode,
        EmojiMode,
        GridMode
    };

    explicit AACKeyboardScreen(AACFramework *framework,
                               AACAccessibilityManager *accessibility,
                               AACInputController *inputController,
                               QWidget *parent = nullptr);

    ~AACKeyboardScreen() override;

signals:
    void characterTyped(const QString &text);
    void backspacePressed();
    void enterPressed();
    void spacePressed();
    void actionTriggered(const QString &action);
    void modeChanged(KeyboardMode mode);

public slots:
    void setMode(KeyboardMode mode);
    void updateCursorContext(int cursorPosition, const QString &text);
    void onFreezeStateChanged(bool frozen);
    void onHighContrastChanged(bool enabled);

    // Scanning
    void onDwellTick();
    void startRowScan();
    void startColumnScan();
    void activateScanTarget();

private slots:
    void handleKeyButtonActivated(const QString &text);
    void handleBackspaceClicked();
    void handleEnterClicked();
    void handleSpaceClicked();

    void handleModeLetters();
    void handleModeNumbers();
    void handleModeSymbols();
    void handleModeEmoji();
    void handleModeGrid();

    void handleEmojiPageLeft();
    void handleEmojiPageRight();

    void updateCursorHighlight(AACKeyButton *btn);

private:
    // UI construction
    void buildUi();
    void buildTopRow();
    void buildKeyboardArea();
    void buildControlRow();

    // Layout builders
    void buildLettersLayout();
    void buildNumbersLayout();
    void buildSymbolsLayout();
    void buildEmojiLayout();
    void buildGridLayout();

    void rebuildKeyboard();
    void clearKeyboardLayout();

    // Visual state
    void applyVisualSettings();
    void updateHighlightForCursor();

    // Scanning helpers
    void clearScanHighlight();
    void highlightScanRow();
    void highlightScanColumn();
    void advanceRowScan();
    void advanceColumnScan();

    // Emoji
    void populateEmojiPages();
    void updateEmojiPage();
    int emojiPageCount() const;

    // Keyboard content
    void populateLettersRows();
    void populateNumbersRows();
    void populateSymbolsRows();
    void populateGridItems();

private:
    AACFramework *m_framework;
    AACAccessibilityManager *m_accessibility;
    AACInputController *m_inputController;

    KeyboardMode m_mode;
    bool m_frozen;
    bool m_highContrast;

    // Layout roots
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_topRowLayout;
    QVBoxLayout *m_keyboardLayout;
    QHBoxLayout *m_controlRowLayout;

    QWidget *m_topRowWidget;
    QWidget *m_keyboardWidget;
    QWidget *m_controlRowWidget;
    QWidget *m_predictiveContainer;

    // Mode buttons
    AACKeyButton *m_lettersModeButton;
    AACKeyButton *m_numbersModeButton;
    AACKeyButton *m_symbolsModeButton;
    AACKeyButton *m_emojiModeButton;
    AACKeyButton *m_gridModeButton;

    // Control row
    AACKeyButton *m_spaceButton;
    AACKeyButton *m_backspaceButton;
    AACKeyButton *m_enterButton;

    // Emoji navigation
    QWidget *m_emojiNavWidget;
    QHBoxLayout *m_emojiNavLayout;
    QPushButton *m_emojiPrevPageButton;
    QPushButton *m_emojiNextPageButton;
    QLabel *m_emojiPageLabel;

    // Keyboard grid
    QGridLayout *m_keyboardGrid;

    // Data
    QList<QStringList> m_lettersRows;
    QList<QStringList> m_numbersRows;
    QList<QStringList> m_symbolsRows;
    QList<QStringList> m_emojiPages;
    QStringList m_gridItems;

    int m_currentEmojiPage;
    int m_cursorPosition;
    QString m_currentText;

    // Highlight tracking
    QPointer<AACKeyButton> m_currentHighlightedButton;

    // Scanning
    bool m_scanning = false;
    int m_scanRow = 0;
    int m_scanCol = -1;
};
