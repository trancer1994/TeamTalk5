#include "AACKeyboardScreen.h"
#include "AACKeyButton.h"
#include "AACFramework.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QSizePolicy>

// =====================================================
//  LETTER + PUNCTUATION VARIANT TABLES (shift-aware)
// =====================================================

static const QHash<QChar, QVector<QChar>> LETTER_VARIANTS = {
    { 'a', { 'á','à','â','ä','ã','å','ā','æ' } },
    { 'b', { 'ƀ','ɓ','β' } },
    { 'c', { 'ç','ć','č','ĉ','ċ' } },
    { 'd', { 'ď','đ','ɗ' } },
    { 'e', { 'é','è','ê','ë','ē','ė','ę' } },
    { 'f', { 'ƒ' } },
    { 'g', { 'ğ','ĝ','ġ','ģ' } },
    { 'h', { 'ĥ','ħ' } },
    { 'i', { 'í','ì','î','ï','ī','į','ı' } },
    { 'j', { 'ĵ' } },
    { 'k', { 'ķ','ĸ' } },
    { 'l', { 'ĺ','ļ','ľ','ŀ','ł' } },
    { 'm', { 'ɱ' } },
    { 'n', { 'ñ','ń','ň','ņ','ŋ' } },
    { 'o', { 'ó','ò','ô','ö','õ','ō','ø','œ' } },
    { 'p', { 'þ','ƥ' } },
    { 'r', { 'ŕ','ř','ŗ' } },
    { 's', { 'ś','š','ş','ŝ','ș' } },
    { 't', { 'ť','ţ','ŧ','ț' } },
    { 'u', { 'ú','ù','û','ü','ū','ů','ű','ų' } },
    { 'w', { 'ŵ' } },
    { 'y', { 'ý','ÿ','ŷ' } },
    { 'z', { 'ź','ž','ż' } }
};

static const QHash<QChar, QVector<QChar>> PUNCT_VARIANTS = {
    { '.', { '…','•','·' } },
    { ',', { '‚','¸' } },
    { '!', { '¡' } },
    { '?', { '¿' } },
    { '"', { '“','”','„','‟' } },
    { '\'', { '‘','’','‚','‛' } },
    { '-', { '–','—','‑' } },
    { '(', { '〔','【','『','（' } },
    { ')', { '〕','】','』','）' } },
    { '/', { '⁄','∕' } },
    { ':', { 'ː','꞉' } },
    { ';', { ';' } }
};

// =====================================================
//  Variant resolver (shift-aware, base included)
// =====================================================

static QVector<QChar> resolveVariants(QChar base, bool shiftOn)
{
    QVector<QChar> out;

    // Always include the base character first
    out.append(shiftOn ? base.toUpper() : base.toLower());

    const QChar lower = base.toLower();

    if (LETTER_VARIANTS.contains(lower)) {
        const QVector<QChar> variants = LETTER_VARIANTS.value(lower);

        if (!shiftOn) {
            out += variants;
        } else {
            QVector<QChar> upper;
            upper.reserve(variants.size());
            for (QChar c : variants)
                upper.append(c.toUpper());
            out += upper;
        }
        return out;
    }

    if (PUNCT_VARIANTS.contains(base)) {
        out += PUNCT_VARIANTS.value(base);
        return out;
    }

    return out;
}

// =====================================================
//  Curated Symbol Strip (always visible)
// =====================================================

static const QStringList CURATED_SYMBOLS = {
    "🙂","😢","😡","😱",
    "👍","👎","❤️","❓",
    "💧","🍽️","🛏️","🆘",
    "👋","🙏","🙇","🤝"
};

// =====================================================
//  Symbol ↔ semantic helpers
// =====================================================

static QString semanticTagForSymbol(const QString& item)
{
    if (item == "🙂") return "emotion_happy";
    if (item == "😢") return "emotion_sad";
    if (item == "😡") return "emotion_angry";
    if (item == "😱") return "emotion_scared";

    if (item == "👍") return "yes";
    if (item == "👎") return "no";
    if (item == "❤️") return "love";
    if (item == "❓") return "question";

    if (item == "💧") return "need_water";
    if (item == "🍽️") return "need_food";
    if (item == "🛏️") return "need_rest";
    if (item == "🆘") return "need_help";

    if (item == "👋") return "hello";
    if (item == "🙏") return "please";
    if (item == "🙇") return "sorry";
    if (item == "🤝") return "thank_you";

    return QString();
}

static QString symbolForSemanticTag(const QString& tag)
{
    const QString t = tag.toLower();

    if (t == "emotion_happy")  return "🙂";
    if (t == "emotion_sad")    return "😢";
    if (t == "emotion_angry")  return "😡";
    if (t == "emotion_scared") return "😱";

    if (t == "yes")      return "👍";
    if (t == "no")       return "👎";
    if (t == "love")     return "❤️";
    if (t == "question") return "❓";

    if (t == "need_water") return "💧";
    if (t == "need_food")  return "🍽️";
    if (t == "need_rest")  return "🛏️";
    if (t == "need_help")  return "🆘";

    if (t == "hello")      return "👋";
    if (t == "please")     return "🙏";
    if (t == "sorry")      return "🙇";
    if (t == "thank_you")  return "🤝";

    return QString();
}
// =====================================================
//  Popup builder using AACKeyButton (with base char)
// =====================================================

QWidget* AACKeyboardScreen::buildPopupForKey(AACKeyButton* btn)
{
    QWidget* popup = new QWidget(this, Qt::Popup);
    popup->setAttribute(Qt::WA_StyledBackground, true);

    popup->setStyleSheet(
        "background: #000000;"
        "border: 3px solid #FFFFFF;"
        "padding: 6px;"
    );

    auto* layout = new QHBoxLayout(popup);
    layout->setSpacing(4);
    layout->setContentsMargins(6, 6, 6, 6);

    const QString text = btn->text();
    if (text.isEmpty())
        return popup;

    const bool shiftOn = m_shift;
    const QVector<QChar> variants = resolveVariants(text.at(0), shiftOn);

    for (QChar v : variants) {
        auto* opt = new AACKeyButton(QString(v), m_accessibility, popup);
        opt->setFixedSize(64, 64);

        connect(opt, &AACKeyButton::keyActivated, this, [this, v, popup]() {
            emit characterTyped(QString(v));
            popup->close();
        });

        layout->addWidget(opt);
    }

    return popup;
}

// =====================================================
//  Constructor / basic wiring
// =====================================================

static QWidget* createSpacer(QWidget* parent)
{
    auto* w = new QWidget(parent);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return w;
}

AACKeyboardScreen::AACKeyboardScreen(AACAccessibilityManager* accessibility,
                                     QWidget* parent)
    : QWidget(parent)
    , m_accessibility(accessibility)
{
    populateLettersRows();
    populateNumbersRows();
    populateSymbolsRows();
    populateGridItems();

    buildUi();
    rebuildKeyboard();
    applyVisualSettings();

QString AACKeyboardScreen::contextualHelp() const
{
    return tr("AACKeyboard. "
               "Type to enter text. "
               "Press F6 to speak your message. "
               "Press Escape to go back.");
}
AACKeyboardScreen::~AACKeyboardScreen() = default;

// =====================================================
//  UI construction
// =====================================================

void AACKeyboardScreen::buildUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Top row
    m_topRowWidget = new QWidget(this);
m_topRowWidget->setObjectName("topRowWidget");
    m_topRowLayout = new QHBoxLayout(m_topRowWidget);
    m_topRowLayout->setContentsMargins(4, 4, 4, 4);
    m_topRowLayout->setSpacing(4);

    buildTopRow();
    m_mainLayout->addWidget(m_topRowWidget);

    // Curated symbol strip (always visible)
    m_curatedStripWidget = new QWidget(this);
m_curatedStripWidget->setObjectName("curatedStripWidget");
    m_curatedStripLayout = new QHBoxLayout(m_curatedStripWidget);
    m_curatedStripLayout->setContentsMargins(4, 0, 4, 0);
    m_curatedStripLayout->setSpacing(4);

    buildCuratedSymbolStrip();
    m_mainLayout->addWidget(m_curatedStripWidget);

    // Keyboard area
    m_keyboardWidget = new QWidget(this);
m_keyboardWidget->setObjectName("keyboardWidget");
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
// --- Auto-scan timer (AAC-native) ---
m_scanTimer = new QTimer(this);
m_scanTimer->setSingleShot(false);

connect(m_scanTimer, &QTimer::timeout,
        this, &AACKeyboardScreen::moveHighlightToNextItem);
}
void AACKeyboardScreen::buildTopRow()
{
    m_lettersModeButton = new AACKeyButton(tr("ABC"), m_accessibility, m_topRowWidget);
m_lettersModeButton->setObjectName("lettersModeButton");
    m_numbersModeButton = new AACKeyButton(tr("123"), m_accessibility, m_topRowWidget);
m_numbersModeButton->setObjectName("numbersModeButton");
    m_symbolsModeButton = new AACKeyButton(tr("#+="), m_accessibility, m_topRowWidget);
m_symbolsModeButton->setObjectName("symbolsModeButton");
    m_gridModeButton    = new AACKeyButton(tr("Grid"), m_accessibility, m_topRowWidget);
m_gridModeButton->setObjectName("gridModeButton");

    connect(m_lettersModeButton, &AACKeyButton::keyActivated,
            this, &AACKeyboardScreen::handleModeLetters);
    connect(m_numbersModeButton, &AACKeyButton::keyActivated,
            this, &AACKeyboardScreen::handleModeNumbers);
    connect(m_symbolsModeButton, &AACKeyButton::keyActivated,
            this, &AACKeyboardScreen::handleModeSymbols);
    connect(m_gridModeButton,    &AACKeyButton::keyActivated,
            this, &AACKeyboardScreen::handleModeGrid);

    m_topRowLayout->addWidget(m_lettersModeButton);
    m_topRowLayout->addWidget(m_numbersModeButton);
    m_topRowLayout->addWidget(m_symbolsModeButton);
    m_topRowLayout->addWidget(m_gridModeButton);
    m_topRowLayout->addWidget(createSpacer(m_topRowWidget));

m_predictiveStrip = new PredictiveStrip(m_topRowWidget);
m_predictiveStrip->setObjectName("predictiveStrip");
m_predictiveStrip->setManager(m_accessibility);
m_topRowLayout->addWidget(m_predictiveStrip, 1);
m_predictiveStrip->setContentsMargins(4, 0, 4, 0);
m_predictiveStrip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

connect(m_predictiveStrip, &PredictiveStrip::suggestionChosen,
        this, &AACKeyboardScreen::predictionChosen);
}
// =====================================================
//  Curated Symbol Strip
// =====================================================

void AACKeyboardScreen::buildCuratedSymbolStrip()
{
    if (!m_curatedStripLayout)
        return;

    // Clear existing
    QLayoutItem* item = nullptr;
    while ((item = m_curatedStripLayout->takeAt(0)) != nullptr) {
        if (auto* w = item->widget())
            w->deleteLater();
        delete item;
    }

    for (const QString& sym : CURATED_SYMBOLS) {
        auto* btn = new AACKeyButton(sym, m_accessibility, m_curatedStripWidget);

        const QString tag = semanticTagForSymbol(sym);
btn->setObjectName("curatedSymbol_" + tag);

        connect(btn, &AACKeyButton::keyActivated, this, [this, sym, tag]() {

    // --- AAC multimodal feedback ---
m_accessibility->feedback()->playAction();

            if (!tag.isEmpty()) {
                emit actionTriggered(tag);
                emit symbolSemantic(tag);
            }
            emit characterTyped(sym + " ");
        });

connect(btn, &AACKeyButton::hovered,
        this, [this, btn]() {
            if (!m_curatedStripLayout)
                return;

            int index = m_curatedStripLayout->indexOf(btn);
            if (index < 0)
                return;

            m_scanRow = 0;      // curated strip row
            m_scanCol = index;

            updateUnifiedHighlight();
        });

        m_curatedStripLayout->addWidget(btn);
    }

    applyVisualSettings();
    emit curatedStripSymbolsChanged(CURATED_SYMBOLS);
}

void AACKeyboardScreen::buildKeyboardArea()
{
    auto* gridContainer = new QWidget(m_keyboardWidget);
    m_keyboardGrid = new QGridLayout(gridContainer);
    m_keyboardGrid->setContentsMargins(0, 0, 0, 0);
    m_keyboardGrid->setSpacing(4);

    m_keyboardLayout->addWidget(gridContainer);
}

void AACKeyboardScreen::buildControlRow()
{
    m_backspaceButton = new AACKeyButton(tr("⌫"), m_accessibility, m_controlRowWidget);
m_backspaceButton->setObjectName("backspaceButton");
    m_spaceButton     = new AACKeyButton(tr("Space"), m_accessibility, m_controlRowWidget);
m_spaceButton->setObjectName("spaceButton");
    m_enterButton     = new AACKeyButton(tr("⏎"), m_accessibility, m_controlRowWidget);
m_enterButton->setObjectName("enterButton");
auto* doneButton  = new AACKeyButton(tr("Done"), m_accessibility, m_controlRowWidget);
doneButton->setObjectName("doneButton");

    connect(m_backspaceButton, &AACKeyButton::keyActivated,
            this, &AACKeyboardScreen::handleBackspaceActivated);
    connect(m_spaceButton, &AACKeyButton::keyActivated,
            this, &AACKeyboardScreen::handleSpaceActivated);
    connect(m_enterButton, &AACKeyButton::keyActivated,
            this, &AACKeyboardScreen::handleEnterActivated);

    m_controlRowLayout->addWidget(m_backspaceButton);
    m_controlRowLayout->addWidget(m_spaceButton, 1);
    m_controlRowLayout->addWidget(m_enterButton);

    // Backspace repeat (non-animated)
    m_backspaceRepeatTimer = new QTimer(this);
    m_backspaceRepeatTimer->setInterval(60);

    connect(m_backspaceRepeatTimer, &QTimer::timeout,
            this, [this]() { emit backspacePressed(); });

    connect(m_backspaceButton, &AACKeyButton::pressed, this, [this]() {
        emit backspacePressed();
        QTimer::singleShot(400, this, [this]() {
m_accessibility->feedback()->playBackspace();
            if (m_backspaceButton->isDown())
                m_backspaceRepeatTimer->start();
        });
    });

    connect(m_backspaceButton, &AACKeyButton::released, this, [this]() {
        m_backspaceRepeatTimer->stop();
    });
}

// =====================================================
//  Mode handling
// =====================================================

void AACKeyboardScreen::setMode(KeyboardMode mode)
{
    if (m_mode == mode)
        return;

    m_mode = mode;
    emit modeChanged(m_mode);

    switch (m_mode) {
    case LettersMode:
        m_cursorPlacement = CursorAfterSpace;          // Proloquo
        break;

    case GridMode:
        m_cursorPlacement = CursorBetweenWordAndSpace; // TD Snap
        break;

    case NumbersMode:
    case SymbolsMode:
        m_cursorPlacement = CursorAfterWord;           // LAMP
        break;
    }

    rebuildKeyboard();
}

void AACKeyboardScreen::handleModeLetters() { setMode(LettersMode); }
void AACKeyboardScreen::handleModeNumbers() { setMode(NumbersMode); }
void AACKeyboardScreen::handleModeSymbols() { setMode(SymbolsMode); }
void AACKeyboardScreen::handleModeGrid()    { setMode(GridMode); }

void AACKeyboardScreen::rebuildKeyboard()
{
    clearKeyboardLayout();

    switch (m_mode) {
    case LettersMode: buildLettersLayout(); break;
    case NumbersMode: buildNumbersLayout(); break;
    case SymbolsMode: buildSymbolsLayout(); break;
    case GridMode:    buildGridLayout();    break;
    }

updateUnifiedHighlight();
}

// =====================================================
//  Layout builders
// =====================================================

void AACKeyboardScreen::buildLettersLayout()
{
    int row = 0;
    for (const QStringList& rowKeys : m_lettersRows) {
        int col = 0;
        const bool isLastRow = (row == m_lettersRows.size() - 1);

        if (isLastRow) {
            auto* leftShift = new AACKeyButton(tr("Shift"), m_accessibility, this);
leftShift->setObjectName("shiftButtonLeft");
            m_shiftButtonLeft = leftShift;
            connect(leftShift, &AACKeyButton::keyActivated,
                    this, &AACKeyboardScreen::toggleShift);
            leftShift->setHighlighted(m_shift || m_capsLock);
            m_keyboardGrid->addWidget(leftShift, row, col++);
        }

        for (const QString& key : rowKeys) {
            auto* btn = new AACKeyButton(key, m_accessibility, this);
btn->setObjectName("key_" + key);

            connect(btn, &AACKeyButton::keyActivated,
                    this, &AACKeyboardScreen::handleKeyButtonActivated);
connect(btn, &AACKeyButton::hovered,
        this, [this, btn]() {
            if (!m_keyboardGrid)
                return;

            int index = m_keyboardGrid->indexOf(btn);
            if (index < 0)
                return;

            int cols = m_keyboardGrid->columnCount();
            int gridRow = index / cols;
            int gridCol = index % cols;

            m_scanRow = gridRow + (m_scanCuratedStrip ? 1 : 0);
            m_scanCol = gridCol;

            updateUnifiedHighlight();
        });

            m_keyboardGrid->addWidget(btn, row, col++);
        }

        if (isLastRow) {
            auto* rightShift = new AACKeyButton(tr("Shift"), m_accessibility, this);
rightShift->setObjectName("shiftButtonRight");
            m_shiftButtonRight = rightShift;
            connect(rightShift, &AACKeyButton::keyActivated,
                    this, &AACKeyboardScreen::toggleShift);
            rightShift->setHighlighted(m_shift || m_capsLock);
            m_keyboardGrid->addWidget(rightShift, row, col++);
        }

        ++row;
    }
}

void AACKeyboardScreen::buildNumbersLayout()
{
    int row = 0;
    for (const QStringList& rowKeys : m_numbersRows) {
        int col = 0;
        for (const QString& key : rowKeys) {
            auto* btn = new AACKeyButton(key, m_accessibility, this);
btn->setObjectName("key_" + key);

            connect(btn, &AACKeyButton::keyActivated,
                    this, &AACKeyboardScreen::handleKeyButtonActivated);
connect(btn, &AACKeyButton::hovered,
        this, [this, btn]() {
            if (!m_keyboardGrid)
                return;

            int index = m_keyboardGrid->indexOf(btn);
            if (index < 0)
                return;

            int cols = m_keyboardGrid->columnCount();
            int gridRow = index / cols;
            int gridCol = index % cols;

            m_scanRow = gridRow + (m_scanCuratedStrip ? 1 : 0);
            m_scanCol = gridCol;

            updateUnifiedHighlight();
        });

            m_keyboardGrid->addWidget(btn, row, col++);
        }
        ++row;
    }
}

void AACKeyboardScreen::buildSymbolsLayout()
{
    int row = 0;
    for (const QStringList& rowKeys : m_symbolsRows) {
        int col = 0;
        for (const QString& key : rowKeys) {
            auto* btn = new AACKeyButton(key, m_accessibility, this);
btn->setObjectName("key_" + key);

            connect(btn, &AACKeyButton::keyActivated,
                    this, &AACKeyboardScreen::handleKeyButtonActivated);
connect(btn, &AACKeyButton::hovered,
        this, [this, btn]() {
            if (!m_keyboardGrid)
                return;

            int index = m_keyboardGrid->indexOf(btn);
            if (index < 0)
                return;

            int cols = m_keyboardGrid->columnCount();
            int gridRow = index / cols;
            int gridCol = index % cols;

            m_scanRow = gridRow + (m_scanCuratedStrip ? 1 : 0);
            m_scanCol = gridCol;

            updateUnifiedHighlight();
        });

            m_keyboardGrid->addWidget(btn, row, col++);
        }
        ++row;
    }
}

void AACKeyboardScreen::buildGridLayout()
{
    int row = 0;
    int col = 0;
    const int columns = 4;

    for (const QString& item : m_gridItems) {
        auto* btn = new AACKeyButton(item, m_accessibility, this);
btn->setObjectName("item_" + item);

    if (item == tr("Clear") || item == tr("Delete word")) {
        btn->setDeepWell(true);
    }
        const QString tag = semanticTagForSymbol(item);

        connect(btn, &AACKeyButton::keyActivated, this, [this, item, tag]() {
m_accessibility->feedback()->playAction();

            if (!tag.isEmpty()) {
                emit actionTriggered(tag);
                emit symbolSemantic(tag);
            } else if (item == tr("Left")) {
                emit moveCursorLeft();
            } else if (item == tr("Right")) {
                emit moveCursorRight();
            } else if (item == tr("Clear")) {
                emit clearRequested();
            } else if (item == tr("Delete word")) {
                emit deleteWordRequested();
            }
        });

connect(btn, &AACKeyButton::hovered,
        this, [this, btn]() {
            if (!m_keyboardGrid)
                return;

            int index = m_keyboardGrid->indexOf(btn);
            if (index < 0)
                return;

            int cols = m_keyboardGrid->columnCount();
            int gridRow = index / cols;
            int gridCol = index % cols;

            m_scanRow = gridRow + (m_scanCuratedStrip ? 1 : 0);
            m_scanCol = gridCol;

            updateUnifiedHighlight();
        });

        m_keyboardGrid->addWidget(btn, row, col);

        if (++col >= columns) {
            col = 0;
            ++row;
        }
    }
}

// =====================================================
//  Predictions (word-based)
// =====================================================

void AACKeyboardScreen::setText(const QString& text)
{
    m_currentText = text;
updateUnifiedHighlight();
}

void AACKeyboardScreen::updateCursorContext(int cursorPosition, const QString& text)
{
    m_cursorPosition = cursorPosition;
    m_currentText = text;

    if (m_predictiveStrip)
        m_predictiveStrip->setContext(currentTokenAtCursor());
updateUnifiedHighlight();
}

void AACKeyboardScreen::applyVisualSettings()
{
    const auto buttons = findChildren<AACKeyButton*>();
    for (AACKeyButton* btn : buttons)
        btn->setHighContrast(m_highContrast);
}

void AACKeyboardScreen::setPredictions(const QStringList& words)
{
    if (!m_predictiveStrip)
        return;

    m_predictiveStrip->setPredictions(words);
}
void AACKeyboardScreen::predictionChosen(const QString& word)
{
    replaceTokenAtCursor(word);

    emit predictionInserted(word);
}

// =====================================================
//  Key handling + spacing + shift
// =====================================================

void AACKeyboardScreen::handleKeyButtonActivated(const QString& text)
{
    if (m_frozen)
        return;

// --- AAC multimodal feedback ---
m_accessibility->feedback()->playClick();

    QString out = applyAutoCapitalization(text);
    out = applySmartSpacing(out);

    if ((m_shift || m_capsLock) && out.size() == 1 && out.at(0).isLetter())
        out = out.toUpper();

    emit characterTyped(out);

    if (m_shift && !m_capsLock)
        toggleShift();
}

void AACKeyboardScreen::handleBackspaceActivated()
{
    if (!m_frozen)
        return;

m_accessibility->feedback()->playBackspace();

        emit backspacePressed();
}

void AACKeyboardScreen::handleEnterActivated()
{
    if (m_frozen)
        return;

m_accessibility->feedback()->playEnter();

    emit enterPressed();
    emit characterTyped("\n");
}

void AACKeyboardScreen::handleSpaceActivated()
{
    if (m_frozen)
        return;

    // Double-space → period + space
    if (m_cursorPosition > 0 && m_cursorPosition <= m_currentText.size()) {
        const QChar prev = m_currentText.at(m_cursorPosition - 1);
        const bool prevIsSpace = (prev == QChar(' '));

        if (prevIsSpace && m_cursorPosition >= 2 &&
            m_currentText.at(m_cursorPosition - 2).isLetterOrNumber()) {

            m_currentText[m_cursorPosition - 1] = '.';
            emit characterTyped(" ");
            return;
        }
    }

m_accessibility->feedback()->playClick();

    emit spacePressed();
}
// =====================================================
//  Auto-capitalization + smart spacing
// =====================================================

bool AACKeyboardScreen::shouldAutoCapitalize(const QString& text, int cursorPos) const
{
    // Start of text → always capitalize
    if (cursorPos == 0)
        return true;

    // --- GRID MODE SPECIAL RULES ---
    if (m_mode == GridMode) {

        // Get the previous token (symbol or word)
        QString prevToken = previousToken(cursorPos);

        // If previous token is a semantic symbol → DO NOT auto-capitalize
        if (!semanticTagForSymbol(prevToken).isEmpty())
            return false;

        // Otherwise only capitalize after sentence-ending punctuation
        const QChar prev = text.at(cursorPos - 1);
        return prev == '.' || prev == '!' || prev == '?' || prev == '\n';
    }

    // --- NORMAL LETTERS MODE RULE ---
    const QChar prev = text.at(cursorPos - 1);
    return prev == '.' || prev == '?' || prev == '!' || prev == '\n';
}

QString AACKeyboardScreen::applyAutoCapitalization(const QString& input) const
{
    if (input.size() == 1 && input.at(0).isLetter()) {
        if (shouldAutoCapitalize(m_currentText, m_cursorPosition))
            return input.toUpper();
    }
    return input;
}

QString AACKeyboardScreen::applySmartSpacing(const QString& typed) const
{
    // Remove space before punctuation
    if (typed.size() == 1 && !m_currentText.isEmpty() && m_cursorPosition > 0) {
        const QChar ch = typed.at(0);
        if (QString(".,!?;:").contains(ch)) {
            if (m_cursorPosition > 0 &&
                m_currentText.at(m_cursorPosition - 1) == QChar(' ')) {
                return QString(ch);
            }
        }
    }
    return typed;
}
QString AACKeyboardScreen::previousToken(int cursorPos) const
{
    if (cursorPos <= 0 || cursorPos > m_currentText.size())
        return QString();

    int end = cursorPos - 1;
    while (end > 0 && m_currentText.at(end).isSpace())
        --end;

    if (end < 0)
        return QString();

    int start = end;
    while (start > 0 && !m_currentText.at(start - 1).isSpace())
        --start;

    return m_currentText.mid(start, end - start + 1);
}
QString AACKeyboardScreen::currentTokenAtCursor() const
{
    if (m_cursorPosition < 0 || m_cursorPosition > m_currentText.size())
        return QString();

    int start = m_cursorPosition;
    int end   = m_cursorPosition;

    // Move start left until space or start of text
    while (start > 0 && !m_currentText.at(start - 1).isSpace())
        --start;

    // Move end right until space or end of text
    while (end < m_currentText.size() && !m_currentText.at(end).isSpace())
        ++end;

    return m_currentText.mid(start, end - start);
}

void AACKeyboardScreen::replaceTokenAtCursor(const QString& replacement)
{
    if (m_cursorPosition < 0 || m_cursorPosition > m_currentText.size())
        return;

    int start = m_cursorPosition;
    int end   = m_cursorPosition;

    // Find token boundaries
    while (start > 0 && !m_currentText.at(start - 1).isSpace())
        --start;

    while (end < m_currentText.size() && !m_currentText.at(end).isSpace())
        ++end;

    QString before = m_currentText.left(start);
    QString after  = m_currentText.mid(end);

    // --- 1. CURSOR-AWARE CAPITALIZATION ---
    QString word = replacement;
    if (shouldAutoCapitalize(m_currentText, start))
        word[0] = word[0].toUpper();

    // --- 2. CURSOR-AWARE PUNCTUATION MERGING ---
    const QChar afterFirst = after.isEmpty() ? QChar() : after.at(0);
    const bool afterIsPunct = QStringLiteral(".,!?;:").contains(afterFirst);

    // If replacement ends with punctuation and after also starts with punctuation → collapse
    if (!after.isEmpty() && afterIsPunct && !word.isEmpty()) {
        const QChar last = word.at(word.size() - 1);
        if (QStringLiteral(".,!?;:").contains(last)) {
            // Remove duplicate punctuation
            if (last == afterFirst)
                after.remove(0, 1);
        }
    }

    // --- 3. CURSOR-AWARE SPACING ---
    const bool beforeHasSpace = !before.isEmpty() && before.endsWith(' ');
    const bool afterHasSpace  = !after.isEmpty() && after.startsWith(' ');

    QString newText = before;

    // Leading space if needed
    if (!beforeHasSpace && !before.isEmpty() && !before.endsWith('\n'))
        newText += ' ';

    int wordStartPos = newText.size();
    newText += word;
    int wordEndPos = newText.size();

    // Trailing space rules
    bool needTrailingSpace = false;

    if (after.isEmpty()) {
        // End of text → always add trailing space
        needTrailingSpace = true;
    } else if (!afterHasSpace && !afterIsPunct) {
        // Next token is a word → add space
        needTrailingSpace = true;
    }

    if (needTrailingSpace)
        newText += ' ';

    int cursorPos = 0;

    // --- 4. CURSOR PLACEMENT MODES ---
    switch (m_cursorPlacement) {
    case CursorAfterSpace: // Proloquo
        cursorPos = newText.size();
        break;

    case CursorAfterWord: // LAMP
        cursorPos = wordEndPos;
        break;

    case CursorBetweenWordAndSpace: // TD Snap
        cursorPos = needTrailingSpace ? wordEndPos : newText.size();
        break;
    }

    newText += after;

    emit replaceText(newText, cursorPos);
}

// =====================================================
//  Shift / CapsLock
// =====================================================

void AACKeyboardScreen::toggleShift()
{
    m_shift = !m_shift;
    emit shiftStateChanged(m_shift);

    if (m_shiftButtonLeft)
        m_shiftButtonLeft->setHighlighted(m_shift || m_capsLock);
    if (m_shiftButtonRight)
        m_shiftButtonRight->setHighlighted(m_shift || m_capsLock);
}

void AACKeyboardScreen::toggleCapsLock()
{
    m_capsLock = !m_capsLock;

    if (m_capsLock && !m_shift)
        m_shift = true;
    else if (!m_capsLock && m_shift)
        m_shift = false;

    emit shiftStateChanged(m_shift);

    if (m_shiftButtonLeft)
        m_shiftButtonLeft->setHighlighted(m_shift || m_capsLock);
    if (m_shiftButtonRight)
        m_shiftButtonRight->setHighlighted(m_shift || m_capsLock);
}

// =====================================================
//  Freeze / high contrast
// =====================================================

void AACKeyboardScreen::onFreezeStateChanged(bool frozen)
{
    m_frozen = frozen;
}

void AACKeyboardScreen::onHighContrastChanged(bool enabled)
{
    m_highContrast = enabled;
    applyVisualSettings();
}

// =====================================================
//  Scanning + highlight + helpers (non-animated)
// =====================================================

void AACKeyboardScreen::onDwellTick()
{
    const auto modes = m_accessibility->modes();

    // Curated strip dwell support (auto scanning)
    if (m_scanning && m_curatedStripDwell && m_scanCuratedStrip && m_scanRow == 0) {

        // Adaptive attenuation / fatigue shaping hook
        if (modes.feedbackEnabled) {
            m_accessibility->feedbackEngine()->applyFatigueShaping();
            m_accessibility->feedbackEngine()->applyScanningAttenuation();
        }

        activateScanTarget();
        return;
    }
}

void AACKeyboardScreen::startRowScan()
{
    const auto modes = m_accessibility->modes();

    // External controller is responsible for TalkBack / VoiceOver / Switch Control
    // suppression; we just honour the scanning flags here.
    if (!modes.scanning)
        return;

    m_scanning = true;

    // Core symbols first
    if (m_coreSymbolsFirst && m_scanCuratedStrip)
        m_scanRow = 0;
    else
        m_scanRow = (m_scanCuratedStrip ? 1 : 0);

    m_scanCol = 0;

    // Row‑level speech + soft haptic
    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playHapticSoft();
        m_accessibility->speechEngine()->speakScanningRow(m_scanRow);
        m_accessibility->feedbackEngine()->applyScanningAttenuation();
    }

    updateUnifiedHighlight();
// --- Auto-scan start ---
if (!modes.stepScanning && m_scanTimer)
    m_scanTimer->start(modes.scanningSpeedMs);

if (m_accessibility && m_accessibility->predictionEngine())
    m_accessibility->predictionEngine()->freezePredictions();
}

void AACKeyboardScreen::startColumnScan()
{
    const auto modes = m_accessibility->modes();
    if (!m_scanning || !modes.scanning)
        return;

    m_scanCol = 0;

    // Item‑level speech + soft haptic
    if (modes.feedbackEnabled) {
        if (auto* btn = highlightedButton()) {
            m_accessibility->feedbackEngine()->playHapticSoft();
            m_accessibility->speechEngine()->speakScanningItem(btn->text());
            m_accessibility->feedbackEngine()->applyScanningAttenuation();
        }
    }

if (m_accessibility && m_accessibility->predictionEngine())
    m_accessibility->predictionEngine()->freezePredictions();

    updateUnifiedHighlight();
}

void AACKeyboardScreen::handleStepNext()
{
    const auto modes = m_accessibility->modes();
    if (!modes.scanning || !modes.stepScanning)
        return;

    moveHighlightToNextItem();

    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playStepAdvance();
        m_accessibility->feedbackEngine()->playHapticSoft();

        if (auto* btn = highlightedButton())
            m_accessibility->speechEngine()->speakScanningItem(btn->text());

        m_accessibility->feedbackEngine()->applyScanningAttenuation();
        m_accessibility->feedbackEngine()->applyFatigueShaping();
    }
if (m_accessibility && m_accessibility->predictionEngine())
    m_accessibility->predictionEngine()->freezePredictions();
}

void AACKeyboardScreen::handleStepPrevious()
{
    const auto modes = m_accessibility->modes();
    if (!modes.scanning || !modes.stepScanning)
        return;

    moveHighlightToPreviousItem();

    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playStepAdvance();
        m_accessibility->feedbackEngine()->playHapticSoft();

        if (auto* btn = highlightedButton())
            m_accessibility->speechEngine()->speakScanningItem(btn->text());

        m_accessibility->feedbackEngine()->applyScanningAttenuation();
        m_accessibility->feedbackEngine()->applyFatigueShaping();
    }
if (m_accessibility && m_accessibility->predictionEngine())
    m_accessibility->predictionEngine()->freezePredictions();
}

void AACKeyboardScreen::handleStepSelect()
{
    const auto modes = m_accessibility->modes();
    if (!modes.scanning || !modes.stepScanning)
        return;

    // Confirm haptic before activation
    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playHapticConfirm();
        m_accessibility->feedbackEngine()->applyFatigueShaping();
    }

    activateScanTarget(); // already plays click/enter/action
if (m_accessibility && m_accessibility->predictionEngine())
    m_accessibility->predictionEngine()->unfreezePredictions();
}

void AACKeyboardScreen::activateScanTarget()
{
    const auto modes = m_accessibility->modes();
    if (!modes.scanning)
        return;

    if (auto* btn = highlightedButton()) {

        const QString text = btn->text();
        const QString tag  = semanticTagForSymbol(text);

        // --- AAC multimodal feedback ---
        if (modes.feedbackEnabled) {

            // Adaptive attenuation / fatigue shaping on activation
            m_accessibility->feedbackEngine()->applyScanningAttenuation();
            m_accessibility->feedbackEngine()->applyFatigueShaping();

            if (btn == m_backspaceButton) {
                m_accessibility->feedback()->playBackspace();
            }
            else if (btn == m_enterButton) {
                m_accessibility->feedback()->playEnter();
            }
            else if (btn->isDeepWell() || !tag.isEmpty()) {
                m_accessibility->feedback()->playAction();
            }
            else {
                m_accessibility->feedback()->playClick();
            }

            // Confirm haptic on successful activation
            m_accessibility->feedbackEngine()->playHapticConfirm();
        }

        // Item speech on activation
        m_accessibility->speechEngine()->speakScanningItem(text);

        emit btn->keyActivated(text);
    }
}

void AACKeyboardScreen::setCoreSymbolsFirst(bool enabled)
{
    m_coreSymbolsFirst = enabled;
}

void AACKeyboardScreen::setCuratedStripDwellEnabled(bool enabled)
{
    m_curatedStripDwell = enabled;
}

void AACKeyboardScreen::setHighContrastEnabled(bool enabled)
{
    m_highContrast = enabled;
    applyVisualSettings();
}

void AACKeyboardScreen::setFreezeEnabled(bool enabled)
{
    m_frozen = enabled;
}

AACKeyButton* AACKeyboardScreen::highlightedButton() const
{
    // --- 1. Curated strip row (row 0) ---
    if (m_scanCuratedStrip && m_scanRow == 0 && m_curatedStripLayout) {
        if (m_scanCol >= 0 && m_scanCol < m_curatedStripLayout->count()) {
            if (auto* item = m_curatedStripLayout->itemAt(m_scanCol))
                return qobject_cast<AACKeyButton*>(item->widget());
        }
        return nullptr;
    }

    // --- 2. Keyboard grid rows (row 1+) ---
    if (!m_keyboardGrid)
        return nullptr;

    // If curated strip is enabled, keyboard rows start at row 1
    int gridRow = m_scanRow - (m_scanCuratedStrip ? 1 : 0);
    if (gridRow < 0)
        return nullptr;

    if (auto* item = m_keyboardGrid->itemAtPosition(gridRow, m_scanCol))
        return qobject_cast<AACKeyButton*>(item->widget());

    return nullptr;
}

void AACKeyboardScreen::updateUnifiedHighlight()
{
    // Clear previous highlight
    if (m_currentHighlightedButton) {
        m_currentHighlightedButton->setHighlighted(false);
        m_currentHighlightedButton.clear();
    }

    // Find new button
    if (auto* btn = highlightedButton()) {
        btn->setHighlighted(true);
        btn->setFocus();
        m_currentHighlightedButton = btn;
    }
}

void AACKeyboardScreen::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {

    case Qt::Key_Left:  moveHighlightLeft();  return;
    case Qt::Key_Right: moveHighlightRight(); return;
    case Qt::Key_Up:    moveHighlightUp();    return;
    case Qt::Key_Down:  moveHighlightDown();  return;

    default:
        QWidget::keyPressEvent(e);
        return;
    }
}

void AACKeyboardScreen::moveHighlightLeft()
{
    const auto modes = m_accessibility->modes();
    m_scanCol = qMax(0, m_scanCol - 1);

    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playHapticSoft();
        if (auto* btn = highlightedButton())
            m_accessibility->speechEngine()->speakScanningItem(btn->text());
        m_accessibility->feedbackEngine()->applyScanningAttenuation();
    }

    updateUnifiedHighlight();
}

void AACKeyboardScreen::moveHighlightRight()
{
    const auto modes = m_accessibility->modes();
    m_scanCol = qMin(maxCol(), m_scanCol + 1);

    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playHapticSoft();
        if (auto* btn = highlightedButton())
            m_accessibility->speechEngine()->speakScanningItem(btn->text());
        m_accessibility->feedbackEngine()->applyScanningAttenuation();
    }

    updateUnifiedHighlight();
}

void AACKeyboardScreen::moveHighlightUp()
{
    const auto modes = m_accessibility->modes();
    m_scanRow = qMax(0, m_scanRow - 1);
    m_scanCol = qMin(m_scanCol, maxCol());

    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playHapticSoft();
        m_accessibility->speechEngine()->speakScanningRow(m_scanRow);
        m_accessibility->feedbackEngine()->applyScanningAttenuation();
    }

    updateUnifiedHighlight();
}

void AACKeyboardScreen::moveHighlightDown()
{
    const auto modes = m_accessibility->modes();
    m_scanRow = qMin(maxRow(), m_scanRow + 1);
    m_scanCol = qMin(m_scanCol, maxCol());

    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playHapticSoft();
        m_accessibility->speechEngine()->speakScanningRow(m_scanRow);
        m_accessibility->feedbackEngine()->applyScanningAttenuation();
    }

    updateUnifiedHighlight();
}

int AACKeyboardScreen::maxRow() const
{
    int rows = 0;

    // Curated strip row (row 0)
    if (m_scanCuratedStrip && m_curatedStripLayout)
        rows += 1;

    // Keyboard grid rows (row 1+)
    if (m_keyboardGrid)
        rows += m_keyboardGrid->rowCount();

    return rows > 0 ? rows - 1 : 0;
}

int AACKeyboardScreen::maxCol() const
{
    return m_keyboardGrid ? m_keyboardGrid->columnCount() - 1 : 0;
}

void AACKeyboardScreen::moveHighlightToNextItem()
{
    const auto modes = m_accessibility->modes();
    if (modes.stepScanning)
        return;   // STOP auto movement when step scanning is active

    int totalCols = m_keyboardGrid->columnCount();
    int totalRows = m_keyboardGrid->rowCount() + 1; // + curated strip row

    int index = m_scanRow * totalCols + m_scanCol;
    index++;

    if (index >= totalRows * totalCols)
        index = 0;

    m_scanRow = index / totalCols;
    m_scanCol = index % totalCols;

    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playHapticSoft();
        if (auto* btn = highlightedButton())
            m_accessibility->speechEngine()->speakScanningItem(btn->text());
        m_accessibility->feedbackEngine()->applyScanningAttenuation();
    }

    updateUnifiedHighlight();
}

void AACKeyboardScreen::moveHighlightToPreviousItem()
{
    const auto modes = m_accessibility->modes();
    if (modes.stepScanning)
        return;   // STOP auto movement when step scanning is active

    int totalCols = m_keyboardGrid->columnCount();
    int totalRows = m_keyboardGrid->rowCount() + 1; // + curated strip row

    int index = m_scanRow * totalCols + m_scanCol;
    index--;

    if (index < 0)
        index = totalRows * totalCols - 1;

    m_scanRow = index / totalCols;
    m_scanCol = index % totalCols;

    if (modes.feedbackEnabled) {
        m_accessibility->feedbackEngine()->playHapticSoft();
        if (auto* btn = highlightedButton())
            m_accessibility->speechEngine()->speakScanningItem(btn->text());
        m_accessibility->feedbackEngine()->applyScanningAttenuation();
    }

    updateUnifiedHighlight();
}
void AACKeyboardScreen::stopScan()
{
    m_scanning = false;
    if (m_scanTimer)
        m_scanTimer->stop();

if (m_accessibility && m_accessibility->predictionEngine())
    m_accessibility->predictionEngine()->unfreezePredictions();

    updateUnifiedHighlight();
}
void AACKeyboardScreen::setSemanticHighlight(const QString& tag)
{
    // Highlight curated strip symbols
    for (int i = 0; i < m_curatedStripLayout->count(); ++i) {
        QWidget* w = m_curatedStripLayout->itemAt(i)->widget();
        if (auto* btn = qobject_cast<AACKeyButton*>(w)) {
            QString sym = btn->text();
            QString symTag = semanticTagForSymbol(sym);
            btn->setHighlighted(symTag == tag);
        }
    }

    // Highlight grid symbols (GridMode)
    if (m_mode == GridMode && m_keyboardGrid) {
        for (int i = 0; i < m_keyboardGrid->count(); ++i) {
            QWidget* w = m_keyboardGrid->itemAt(i)->widget();
            if (auto* btn = qobject_cast<AACKeyButton*>(w)) {
                QString sym = btn->text();
                QString symTag = semanticTagForSymbol(sym);
                btn->setHighlighted(symTag == tag);
            }
        }
    }
}
