#include "AACKeyboardScreen.h"
#include "AACKeyButton.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "AACFramework.h"
#include "AACAccessibilityManager.h"
#include "AACInputController.h"

AACKeyboardScreen::AACKeyboardScreen(AACFramework *framework,
                                     AACAccessibilityManager *accessibility,
                                     AACInputController *inputController,
                                     QWidget *parent)
    : QWidget(parent),
      m_framework(framework),
      m_accessibility(accessibility),
      m_inputController(inputController),
      m_mode(LettersMode),
      m_frozen(false),
      m_highContrast(false),
      m_mainLayout(nullptr),
      m_topRowLayout(nullptr),
      m_keyboardLayout(nullptr),
      m_controlRowLayout(nullptr),
      m_topRowWidget(nullptr),
      m_keyboardWidget(nullptr),
      m_controlRowWidget(nullptr),
      m_predictiveContainer(nullptr),
      m_lettersModeButton(nullptr),
      m_numbersModeButton(nullptr),
      m_symbolsModeButton(nullptr),
      m_emojiModeButton(nullptr),
      m_gridModeButton(nullptr),
      m_spaceButton(nullptr),
      m_backspaceButton(nullptr),
      m_enterButton(nullptr),
      m_emojiNavWidget(nullptr),
      m_emojiNavLayout(nullptr),
      m_emojiPrevPageButton(nullptr),
      m_emojiNextPageButton(nullptr),
      m_emojiPageLabel(nullptr),
      m_keyboardGrid(nullptr),
      m_currentEmojiPage(0),
      m_cursorPosition(0)
{
    populateLettersRows();
    populateNumbersRows();
    populateSymbolsRows();
    populateEmojiPages();
    populateGridItems();

    buildUi();
    rebuildKeyboard();
    applyVisualSettings();
}

AACKeyboardScreen::~AACKeyboardScreen() = default;

void AACKeyboardScreen::setMode(AACKeyboardScreen::KeyboardMode mode)
{
    if (m_mode == mode)
        return;

    m_mode = mode;
    emit modeChanged(m_mode);
    rebuildKeyboard();
}

void AACKeyboardScreen::updateCursorContext(int cursorPosition, const QString &text)
{
    m_cursorPosition = cursorPosition;
    m_currentText = text;
    updateHighlightForCursor();
}

void AACKeyboardScreen::updateCursorHighlight(AACKeyButton *btn)
{
    if (m_currentHighlightedButton)
        m_currentHighlightedButton->setHighlighted(false);

    m_currentHighlightedButton = btn;
    btn->setHighlighted(true);
}

void AACKeyboardScreen::applyVisualSettings()
{
    const auto buttons = findChildren<AACKeyButton*>();
    for (AACKeyButton *btn : buttons)
        btn->setHighContrast(m_highContrast);
}

static QWidget *createSpacer(QWidget *parent)
{
    QWidget *w = new QWidget(parent);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return w;
}

void AACKeyboardScreen::buildUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Top row
    m_topRowWidget = new QWidget(this);
    m_topRowLayout = new QHBoxLayout(m_topRowWidget);
    m_topRowLayout->setContentsMargins(4, 4, 4, 4);
    m_topRowLayout->setSpacing(4);

    buildTopRow();
    m_mainLayout->addWidget(m_topRowWidget);

    // Keyboard area
    m_keyboardWidget = new QWidget(this);
    m_keyboardLayout = new QVBoxLayout(m_keyboardWidget);
    m_keyboardLayout->setContentsMargins(4, 0, 4, 0);
    m_keyboardLayout->setSpacing(4);

    buildKeyboardArea();
    m_mainLayout->addWidget(m_keyboardWidget, 1);

    // Control row
    m_controlRowWidget = new QWidget(this);
    m_controlRowLayout = new QHBoxLayout(m_controlRowWidget);
    m_controlRowLayout->setContentsMargins(4, 4, 4, 4);
    m_controlRowLayout->setSpacing(4);

    buildControlRow();
    m_mainLayout->addWidget(m_controlRowWidget);
}

void AACKeyboardScreen::buildTopRow()
{
    m_lettersModeButton = new AACKeyButton(tr("ABC"), m_accessibility, m_topRowWidget);
    m_numbersModeButton = new AACKeyButton(tr("123"), m_accessibility, m_topRowWidget);
    m_symbolsModeButton = new AACKeyButton(tr("#+="), m_accessibility, m_topRowWidget);
    m_emojiModeButton   = new AACKeyButton(tr("😊"), m_accessibility, m_topRowWidget);
    m_gridModeButton    = new AACKeyButton(tr("Grid"), m_accessibility, m_topRowWidget);

    connect(m_lettersModeButton, &QPushButton::clicked, this, &AACKeyboardScreen::handleModeLetters);
    connect(m_numbersModeButton, &QPushButton::clicked, this, &AACKeyboardScreen::handleModeNumbers);
    connect(m_symbolsModeButton, &QPushButton::clicked, this, &AACKeyboardScreen::handleModeSymbols);
    connect(m_emojiModeButton,   &QPushButton::clicked, this, &AACKeyboardScreen::handleModeEmoji);
    connect(m_gridModeButton,    &QPushButton::clicked, this, &AACKeyboardScreen::handleModeGrid);

    m_topRowLayout->addWidget(m_lettersModeButton);
    m_topRowLayout->addWidget(m_numbersModeButton);
    m_topRowLayout->addWidget(m_symbolsModeButton);
    m_topRowLayout->addWidget(m_emojiModeButton);
    m_topRowLayout->addWidget(m_gridModeButton);
    m_topRowLayout->addWidget(createSpacer(m_topRowWidget));

    m_predictiveContainer = new QWidget(m_topRowWidget);
    m_predictiveContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_topRowLayout->addWidget(m_predictiveContainer);
}

void AACKeyboardScreen::buildKeyboardArea()
{
    QWidget *gridContainer = new QWidget(m_keyboardWidget);
    m_keyboardGrid = new QGridLayout(gridContainer);
    m_keyboardGrid->setContentsMargins(0, 0, 0, 0);
    m_keyboardGrid->setSpacing(4);

    m_keyboardLayout->addWidget(gridContainer);

    // Emoji navigation
    m_emojiNavWidget = new QWidget(m_keyboardWidget);
    m_emojiNavLayout = new QHBoxLayout(m_emojiNavWidget);
    m_emojiNavLayout->setContentsMargins(0, 0, 0, 0);
    m_emojiNavLayout->setSpacing(4);

    m_emojiPrevPageButton = new QPushButton(tr("◀"), m_emojiNavWidget);
    m_emojiNextPageButton = new QPushButton(tr("▶"), m_emojiNavWidget);
    m_emojiPageLabel      = new QLabel(m_emojiNavWidget);

    connect(m_emojiPrevPageButton, &QPushButton::clicked, this, &AACKeyboardScreen::handleEmojiPageLeft);
    connect(m_emojiNextPageButton, &QPushButton::clicked, this, &AACKeyboardScreen::handleEmojiPageRight);

    m_emojiNavLayout->addWidget(m_emojiPrevPageButton);
    m_emojiNavLayout->addWidget(m_emojiPageLabel, 1);
    m_emojiNavLayout->addWidget(m_emojiNextPageButton);

    m_keyboardLayout->addWidget(m_emojiNavWidget);
    m_emojiNavWidget->setVisible(false);
}

void AACKeyboardScreen::handleKeyButtonActivated(const QString &text)
{
    if (!m_frozen)
        emit characterTyped(text);
}

void AACKeyboardScreen::buildControlRow()
{
    m_spaceButton = new AACKeyButton(tr("Space"), m_accessibility, m_controlRowWidget);
    m_backspaceButton = new AACKeyButton(tr("⌫"), m_accessibility, m_controlRowWidget);
    m_enterButton = new AACKeyButton(tr("⏎"), m_accessibility, m_controlRowWidget);

    connect(m_spaceButton, &QPushButton::clicked, this, &AACKeyboardScreen::handleSpaceClicked);
    connect(m_backspaceButton, &QPushButton::clicked, this, &AACKeyboardScreen::handleBackspaceClicked);
    connect(m_enterButton, &QPushButton::clicked, this, &AACKeyboardScreen::handleEnterClicked);

    m_controlRowLayout->addWidget(m_backspaceButton);
    m_controlRowLayout->addWidget(m_spaceButton, 1);
    m_controlRowLayout->addWidget(m_enterButton);
}

void AACKeyboardScreen::rebuildKeyboard()
{
    clearKeyboardLayout();
    m_emojiNavWidget->setVisible(false);

    switch (m_mode) {
    case LettersMode: buildLettersLayout(); break;
    case NumbersMode: buildNumbersLayout(); break;
    case SymbolsMode: buildSymbolsLayout(); break;
    case EmojiMode:   buildEmojiLayout();   break;
    case GridMode:    buildGridLayout();    break;
    }

    updateHighlightForCursor();
}

void AACKeyboardScreen::clearKeyboardLayout()
{
    if (!m_keyboardGrid)
        return;

    while (QLayoutItem *item = m_keyboardGrid->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }
}

void AACKeyboardScreen::buildLettersLayout()
{
    int row = 0;
    for (const QStringList &rowKeys : m_lettersRows) {
        int col = 0;
        for (const QString &key : rowKeys) {
            AACKeyButton *btn = new AACKeyButton(key, m_accessibility, this);

            connect(btn, &AACKeyButton::keyActivated,
                    this, &AACKeyboardScreen::handleKeyButtonActivated);
            connect(btn, &AACKeyButton::hovered,
                    this, &AACKeyboardScreen::updateCursorHighlight);

            m_keyboardGrid->addWidget(btn, row, col++);
        }
        ++row;
    }
}

void AACKeyboardScreen::buildNumbersLayout()
{
    int row = 0;
    for (const QStringList &rowKeys : m_numbersRows) {
        int col = 0;
        for (const QString &key : rowKeys) {
            AACKeyButton *btn = new AACKeyButton(key, m_accessibility, this);

            connect(btn, &AACKeyButton::keyActivated,
                    this, &AACKeyboardScreen::handleKeyButtonActivated);
            connect(btn, &AACKeyButton::hovered,
                    this, &AACKeyboardScreen::updateCursorHighlight);

            m_keyboardGrid->addWidget(btn, row, col++);
        }
        ++row;
    }
}

void AACKeyboardScreen::buildSymbolsLayout()
{
    int row = 0;
    for (const QStringList &rowKeys : m_symbolsRows) {
        int col = 0;
        for (const QString &key : rowKeys) {
            AACKeyButton *btn = new AACKeyButton(key, m_accessibility, this);

            connect(btn, &AACKeyButton::keyActivated,
                    this, &AACKeyboardScreen::handleKeyButtonActivated);
            connect(btn, &AACKeyButton::hovered,
                    this, &AACKeyboardScreen::updateCursorHighlight);

            m_keyboardGrid->addWidget(btn, row, col++);
        }
        ++row;
    }
}

void AACKeyboardScreen::buildEmojiLayout()
{
    m_emojiNavWidget->setVisible(true);
    updateEmojiPage();
}

void AACKeyboardScreen::buildGridLayout()
{
    int row = 0;
    int col = 0;
    const int columns = 4;

    for (const QString &item : m_gridItems) {
        AACKeyButton *btn = new AACKeyButton(item, m_accessibility, this);

        connect(btn, &AACKeyButton::keyActivated,
                this, [this, item](const QString &) {
                    emit actionTriggered(item);
                });
        connect(btn, &AACKeyButton::hovered,
                this, &AACKeyboardScreen::updateCursorHighlight);

        m_keyboardGrid->addWidget(btn, row, col);

        if (++col >= columns) {
            col = 0;
            ++row;
        }
    }
}
void AACKeyboardScreen::updateHighlightForCursor()
{
    if (m_currentHighlightedButton) {
        m_currentHighlightedButton->setHighlighted(false);
        m_currentHighlightedButton.clear();
    }

    if (m_mode == LettersMode ||
        m_mode == NumbersMode ||
        m_mode == SymbolsMode)
    {
        if (m_spaceButton) {
            m_spaceButton->setHighlighted(true);
            m_currentHighlightedButton = m_spaceButton;
        }
    }
}

void AACKeyboardScreen::onDwellTick()
{
    if (!m_scanning)
        return;

    if (m_scanCol < 0)
        advanceRowScan();
    else
        advanceColumnScan();
}

void AACKeyboardScreen::clearScanHighlight()
{
    if (!m_keyboardGrid)
        return;

    for (int r = 0; r < m_keyboardGrid->rowCount(); ++r) {
        for (int c = 0; c < m_keyboardGrid->columnCount(); ++c) {
            if (QLayoutItem *item = m_keyboardGrid->itemAtPosition(r, c)) {
                if (AACKeyButton *btn = qobject_cast<AACKeyButton*>(item->widget()))
                    btn->setHighlighted(false);
            }
        }
    }
}

void AACKeyboardScreen::startRowScan()
{
    if (!m_keyboardGrid)
        return;

    m_scanning = true;
    m_scanRow = 0;
    m_scanCol = -1;
    highlightScanRow();
}

void AACKeyboardScreen::highlightScanRow()
{
    clearScanHighlight();

    for (int c = 0; c < m_keyboardGrid->columnCount(); ++c) {
        if (QLayoutItem *item = m_keyboardGrid->itemAtPosition(m_scanRow, c)) {
            if (AACKeyButton *btn = qobject_cast<AACKeyButton*>(item->widget()))
                btn->setHighlighted(true);
        }
    }
}

void AACKeyboardScreen::advanceRowScan()
{
    if (!m_scanning || !m_keyboardGrid)
        return;

    m_scanRow++;
    if (m_scanRow >= m_keyboardGrid->rowCount())
        m_scanRow = 0;

    highlightScanRow();
}

void AACKeyboardScreen::startColumnScan()
{
    if (!m_keyboardGrid)
        return;

    m_scanCol = 0;
    highlightScanColumn();
}

void AACKeyboardScreen::highlightScanColumn()
{
    clearScanHighlight();

    if (QLayoutItem *item = m_keyboardGrid->itemAtPosition(m_scanRow, m_scanCol)) {
        if (AACKeyButton *btn = qobject_cast<AACKeyButton*>(item->widget()))
            btn->setHighlighted(true);
    }
}

void AACKeyboardScreen::advanceColumnScan()
{
    if (!m_keyboardGrid)
        return;

    m_scanCol++;
    if (m_scanCol >= m_keyboardGrid->columnCount())
        m_scanCol = 0;

    highlightScanColumn();
}

void AACKeyboardScreen::activateScanTarget()
{
    if (!m_keyboardGrid)
        return;

    if (QLayoutItem *item = m_keyboardGrid->itemAtPosition(m_scanRow, m_scanCol)) {
        if (AACKeyButton *btn = qobject_cast<AACKeyButton*>(item->widget()))
            btn->click();
    }
}
void AACKeyboardScreen::onFreezeStateChanged(bool frozen)
{
    m_frozen = frozen;
    setEnabled(!m_frozen);
}

void AACKeyboardScreen::onHighContrastChanged(bool enabled)
{
    m_highContrast = enabled;
    applyVisualSettings();
}

void AACKeyboardScreen::handleBackspaceClicked()
{
    if (!m_frozen)
        emit backspacePressed();
}

void AACKeyboardScreen::handleEnterClicked()
{
    if (!m_frozen)
        emit enterPressed();
}

void AACKeyboardScreen::handleSpaceClicked()
{
    if (!m_frozen)
        emit spacePressed();
}

void AACKeyboardScreen::handleModeLetters()
{
    setMode(LettersMode);
}

void AACKeyboardScreen::handleModeNumbers()
{
    setMode(NumbersMode);
}

void AACKeyboardScreen::handleModeSymbols()
{
    setMode(SymbolsMode);
}

void AACKeyboardScreen::handleModeEmoji()
{
    setMode(EmojiMode);
}

void AACKeyboardScreen::handleModeGrid()
{
    setMode(GridMode);
}

void AACKeyboardScreen::handleEmojiPageLeft()
{
    if (m_currentEmojiPage > 0) {
        --m_currentEmojiPage;
        updateEmojiPage();
    }
}

void AACKeyboardScreen::handleEmojiPageRight()
{
    if (m_currentEmojiPage + 1 < emojiPageCount()) {
        ++m_currentEmojiPage;
        updateEmojiPage();
    }
}

void AACKeyboardScreen::populateLettersRows()
{
    m_lettersRows.clear();

    m_lettersRows << (QStringList()
                      << "Q" << "W" << "E" << "R" << "T" << "Y" << "U" << "I" << "O" << "P");

    m_lettersRows << (QStringList()
                      << "A" << "S" << "D" << "F" << "G" << "H" << "J" << "K" << "L");

    m_lettersRows << (QStringList()
                      << "Z" << "X" << "C" << "V" << "B" << "N" << "M");
}

void AACKeyboardScreen::populateNumbersRows()
{
    m_numbersRows.clear();

    m_numbersRows << (QStringList()
                      << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0");

    m_numbersRows << (QStringList()
                      << "-" << "/" << ":" << ";" << "(" << ")" << "£" << "&" << "@");

    m_numbersRows << (QStringList()
                      << "\"" << "." << "," << "?" << "!" << "'");
}

void AACKeyboardScreen::populateSymbolsRows()
{
    m_symbolsRows.clear();

    m_symbolsRows << (QStringList()
                      << "[" << "]" << "{" << "}" << "#" << "%" << "^" << "*" << "+");

    m_symbolsRows << (QStringList()
                      << "_" << "\\" << "|" << "~" << "<" << ">" << "=");
}

void AACKeyboardScreen::populateEmojiPages()
{
    m_emojiPages.clear();

    // Page 1 — Faces
    QStringList faces = {
        "😀","😁","😂","🤣","😃","😄","😅","😆",
        "😉","😊","😋","😎","😍","😘","😗","😙",
        "😚","🙂","🤗","🤔","🤨","😐","😑","😶",
        "🙄","😏","😣","😖","😫","😩","😢","😭"
    };

    // Page 2 — Hands / Gestures
    QStringList hands = {
        "👍","👎","👊","✊","🤛","🤜","👋","🤚",
        "✋","🖐","🤙","🤞","🤟","🤘","🤌","🤏",
        "👈","👉","👆","👇","☝️","✌️","🤝","🙏"
    };

    // Page 3 — Objects
    QStringList objects = {
        "🎵","🎶","🎤","🎧","🎼","🎹","🎷","🎺",
        "🎸","🥁","📱","💻","🖥️","⌨️","🖱️","💡",
        "🔦","🔋","🔌","⏰","⏱️","⏲️","🕰️","📷",
        "🎥","📹","📼","💿","📀"
    };

    // Page 4 — Symbols
    QStringList symbols = {
        "❤️","🧡","💛","💚","💙","💜","🖤","🤍",
        "🤎","💔","❣️","💕","💞","💓","💗","💖",
        "💘","💝","⭐","🌟","✨","⚡","🔥","💥",
        "❄️","☀️","☁️","🌈","✔️","✖️","➕","➖",
        "➡️","⬅️","⬆️","⬇️","⚠️","❗","❕","❓",
        "❔","🔒","🔓","🔑"
    };

    m_emojiPages << faces << hands << objects << symbols;
    m_currentEmojiPage = 0;
}

void AACKeyboardScreen::populateGridItems()
{
    m_gridItems.clear();

    m_gridItems << tr("Yes")
                << tr("No")
                << tr("Maybe")
                << tr("Help")
                << tr("Stop")
                << tr("More")
                << tr("Less")
                << tr("Thank you");
}
void AACKeyboardScreen::updateEmojiPage()
{
    clearKeyboardLayout();

    if (m_currentEmojiPage < 0 || m_currentEmojiPage >= m_emojiPages.size())
        return;

    const QStringList &page = m_emojiPages.at(m_currentEmojiPage);

    int row = 0;
    int col = 0;
    const int columns = 8;

    for (const QString &emoji : page) {
        AACKeyButton *btn = new AACKeyButton(emoji, m_accessibility, this);

        connect(btn, &AACKeyButton::keyActivated,
                this, &AACKeyboardScreen::handleKeyButtonActivated);

        connect(btn, &AACKeyButton::hovered,
                this, &AACKeyboardScreen::updateCursorHighlight);

        m_keyboardGrid->addWidget(btn, row, col);

        if (++col >= columns) {
            col = 0;
            ++row;
        }
    }

    const int pageIndex = m_currentEmojiPage + 1;
    const int pageTotal = emojiPageCount();
    m_emojiPageLabel->setText(tr("Emoji %1 / %2").arg(pageIndex).arg(pageTotal));
}

int AACKeyboardScreen::emojiPageCount() const
{
    return m_emojiPages.size();
}
