#include "AACKeyboardScreen.h"
#include "AACKeyButton.h"
#include "AACFramework.h"
#include "AACInputController.h"
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

static QChar punctuationForSymbol(const QString& sym)
{
    if (sym == "❓") return '?';
    if (sym == "❤️") return '.';
    if (sym == "👋") return QChar(); // no direct punctuation, just semantic
    if (sym == "😢") return QChar();
    return QChar();
}
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
m_inputController = new AACInputController(m_accessibility, this);

    populateLettersRows();
    populateNumbersRows();
    populateSymbolsRows();
    populateGridItems();

    buildUi();
    rebuildKeyboard();
    applyVisualSettings();

m_inputController->setKeyboardLayout(m_keyboardGrid);

    QVector<QVector<QWidget*>> layoutVector = m_inputController->extractLayoutVector(m_keyboardGrid);
m_accessibility->setKeyboardScanningLayout(layoutVector);

connect(m_inputController, &AACInputController::highlightChanged,
        this, &AACKeyboardScreen::updateUnifiedHighlight);

connect(m_inputController, &AACInputController::activationRequested,
        this, [this](AACKeyButton* btn) {
            handleKeyButtonActivated(btn->text());
        });

connect(m_inputController, &AACInputController::semanticHighlightChanged,
        this, &AACKeyboardScreen::setSemanticHighlight);

QString AACKeyboardScreen::contextualHelp() const
{
    return tr("AACKeyboard. "
               "Type to enter text. "
               "Press F6 to speak your message. "
               "Press Escape to go back.");
}
AACKeyboardScreen::~AACKeyboardScreen() = default;

void AACKeyboardScreen::populateLettersRows()
{
    m_lettersRows.clear();

    // Lowercase QWERTY, three rows
    m_lettersRows << (QStringList()
                      << "q" << "w" << "e" << "r" << "t" << "y" << "u" << "i" << "o" << "p");

    m_lettersRows << (QStringList()
                      << "a" << "s" << "d" << "f" << "g" << "h" << "j" << "k" << "l");

    m_lettersRows << (QStringList()
                      << "z" << "x" << "c" << "v" << "b" << "n" << "m");
}

void AACKeyboardScreen::populateNumbersRows()
{
    m_numbersRows.clear();

    // Numbers row
    m_numbersRows << (QStringList()
                      << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0");

    // Common symbols / currency / punctuation
    m_numbersRows << (QStringList()
                      << "-" << "/" << ":" << ";" << "(" << ")" << "£" << "&" << "@");

    m_numbersRows << (QStringList()
                      << "\"" << "." << "," << "?" << "!" << "'");
}

void AACKeyboardScreen::populateSymbolsRows()
{
    m_symbolsRows.clear();

    // Brackets, math, misc
    m_symbolsRows << (QStringList()
                      << "[" << "]" << "{" << "}" << "#" << "%" << "^" << "*" << "+");

    m_symbolsRows << (QStringList()
                      << "_" << "\\" << "|" << "~" << "<" << ">" << "=");
}

void AACKeyboardScreen::populateGridItems()
{
    m_gridItems.clear();

    // Emotions
    m_gridItems << "🙂" << "😢" << "😡" << "😱";

    // Social / yes-no / question
    m_gridItems << "👍" << "👎" << "❤️" << "❓";

    // Needs
    m_gridItems << "💧" << "🍽️" << "🛏️" << "🆘";

    // Conversation
    m_gridItems << "👋" << "🙏" << "🙇" << "🤝";

    // Editing / navigation (semantic actions)
    m_gridItems << tr("Left")
                << tr("Right")
                << tr("Clear")
                << tr("Delete word")
                << tr("Clear sentence");
}
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

AAC::setElementHelp(m_lettersModeButton, tr("Switch to letters"));
AAC::setElementHelp(m_numbersModeButton, tr("Switch to numbers"));
AAC::setElementHelp(m_symbolsModeButton, tr("Switch to symbols"));
AAC::setElementHelp(m_gridModeButton,    tr("Switch to grid symbols"));
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

connect(btn, &AACKeyButton::keyActivated,
        this, [this, btn]() {
            m_inputController->handleActivation(btn);
        });

connect(btn, &AACKeyButton::hovered,
        this, [this, btn]() {
            m_inputController->handleHoverKey(btn);
        });

AAC::setSemanticTag(btn, tag);
AAC::setElementHelp(btn, tr("Symbol: %1").arg(sym));
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
connect(doneButton, &AACKeyButton::keyActivated,
        this, &AACKeyboardScreen::doneRequested);

    m_controlRowLayout->addWidget(m_backspaceButton);
    m_controlRowLayout->addWidget(m_spaceButton, 1);
    m_controlRowLayout->addWidget(m_enterButton);
m_controlRowLayout->addWidget(doneButton);

    // Backspace repeat (non-animated)
    m_backspaceRepeatTimer = new QTimer(this);
    m_backspaceRepeatTimer->setInterval(60);

    connect(m_backspaceRepeatTimer, &QTimer::timeout,
            this, [this]() { emit backspacePressed(); });

    connect(m_backspaceButton, &AACKeyButton::pressed, this, [this]() {
        emit backspacePressed();
        QTimer::singleShot(400, this, [this]() {
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
        m_cursorPlacement = CursorAfterWord;           // LAMP
        break;

    case SymbolsMode:
    m_cursorPlacement = CursorAfterPunctuation;
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
connect(btn, &AACKeyButton::hovered,
        this, [this, btn]() {
            m_inputController->handleHoverKey(btn);
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
            m_inputController->handleHoverKey(btn);
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
            m_inputController->handleHoverKey(btn);
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

connect(btn, &AACKeyButton::keyActivated,
    this, [this, btn]() {
        m_inputController->handleActivation(btn);
    });
// ⭐ AAC metadata
AAC::setSemanticTag(btn, tag);
AAC::setElementHelp(btn, tr("Symbol: %1").arg(item));
AAC::setRole(btn, "gridSymbol");

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
    // If cursor at start → nothing to delete
    if (m_cursorPosition <= 0)
        return;

    const int pos = m_cursorPosition;
    const QString& t = m_currentText;

    // --- 1. DELETE SPACE AFTER PUNCTUATION AS A UNIT (".␣" → ".")
    if (pos >= 2 &&
        t.at(pos - 1) == ' ' &&
        QStringLiteral(".,!?;:").contains(t.at(pos - 2))) {

        emit backspacePressed(); // delete the space only
        return;
    }

    // --- 2. UNDO DOUBLE-SPACE PERIOD INSERTION ("word.␣" → "word␣")
    if (pos >= 2 &&
        t.at(pos - 1) == ' ' &&
        t.at(pos - 2) == '.') {

        // Delete the space
        emit backspacePressed();

        // Replace the period with a space
        m_currentText[pos - 2] = ' ';
        return;
    }

    // --- 3. GRID MODE: delete whole symbol/token
    if (m_mode == GridMode) {
        QString prevToken = previousToken(pos);
        if (!prevToken.isEmpty() && !semanticTagForSymbol(prevToken).isEmpty()) {
            emit deleteWordRequested(); // your existing signal
            return;
        }
    }

    // --- 4. FALLBACK: normal backspace
    emit backspacePressed();
}

void AACKeyboardScreen::handleEnterActivated()
{
    emit enterPressed();
    emit characterTyped("\n");
}

void AACKeyboardScreen::handleSpaceActivated()
{
    // --- 1. DOUBLE-SPACE → period insertion (word␣␣ → word.␣)
    if (m_cursorPosition > 0 && m_cursorPosition <= m_currentText.size()) {

        const QChar prev = m_currentText.at(m_cursorPosition - 1);

        if (prev == ' ' && m_cursorPosition >= 2) {
            QChar beforeSpace = m_currentText.at(m_cursorPosition - 2);

            if (beforeSpace.isLetterOrNumber()) {
                // Replace previous space with period, then insert a space
                m_currentText[m_cursorPosition - 1] = '.';
                emit characterTyped(" ");
                return;
            }
        }
    }

    // --- 2. TRIPLE-SPACE → literal spacing (word.␣␣␣ → word.␣␣)
    if (m_cursorPosition >= 2 &&
        m_currentText.at(m_cursorPosition - 1) == ' ' &&
        m_currentText.at(m_cursorPosition - 2) == '.') {

        emit characterTyped(" ");
        return;
    }

    // --- 3. AUTO-SPACING AFTER PUNCTUATION (.,!?;:)
    if (m_cursorPosition > 0) {
        const QChar prev = m_currentText.at(m_cursorPosition - 1);

        if (QStringLiteral(".,!?;:").contains(prev)) {

            // If next char is already a space, do nothing special
            if (m_cursorPosition < m_currentText.size() &&
                m_currentText.at(m_cursorPosition) == ' ') {

                emit spacePressed();
                return;
            }

            // Insert a single space after punctuation
            emit characterTyped(" ");
            return;
        }
    }

    // --- 4. FALLBACK: normal space behaviour
    emit spacePressed();
}
// =====================================================
//  Auto-capitalization + smart spacing
// =====================================================

bool AACKeyboardScreen::shouldAutoCapitalize(const QString& text, int cursorPos) const
{
    if (cursorPos <= 0)
        return true;

    // --- GRID MODE ---
    if (m_mode == GridMode) {

        QString prevToken = previousToken(cursorPos);

        if (!semanticTagForSymbol(prevToken).isEmpty())
            return false;

        QChar prev = text.at(cursorPos - 1);
        if (prev == '.' || prev == '!' || prev == '?' || prev == '\n')
            return true;

        // punctuation before spaces
        if (prev.isSpace()) {
            int i = cursorPos - 1;
            while (i > 0 && text.at(i).isSpace())
                --i;

            if (i >= 0) {
                QChar beforeSpace = text.at(i);
                if (beforeSpace == '.' || beforeSpace == '?' || beforeSpace == '!')
                    return true;
            }
        }

        return false;
    }

    // --- LETTERS MODE ---
    QChar prev = text.at(cursorPos - 1);

    if (prev == '.' || prev == '?' || prev == '!' || prev == '\n')
        return true;

    if (prev.isSpace()) {
        int i = cursorPos - 1;
        while (i > 0 && text.at(i).isSpace())
            --i;

        if (i >= 0) {
            QChar beforeSpace = text.at(i);
            if (beforeSpace == '.' || beforeSpace == '?' || beforeSpace == '!')
                return true;
        }
    }

    return false;
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

case CursorAfterPunctuation:
{
    // If the replacement ends with punctuation, place cursor immediately after it.
    if (!word.isEmpty()) {
        QChar last = word.at(word.size() - 1);
        if (QStringLiteral(".,!?;:").contains(last)) {
            cursorPos = wordEndPos;   // right after punctuation
            break;
        }
    }

    // Otherwise behave like CursorAfterSpace
    cursorPos = newText.size();
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

AACKeyButton* AACKeyboardScreen::highlightedButton() const
{
    auto [row, col] = m_inputController->scanPosition();
    bool curated = m_inputController->scanCuratedStripEnabled();

    // --- 1. Curated strip row (row 0) ---
    if (curated && row == 0 && m_curatedStripLayout) {
        if (col >= 0 && col < m_curatedStripLayout->count()) {
            if (auto* item = m_curatedStripLayout->itemAt(col))
                return qobject_cast<AACKeyButton*>(item->widget());
        }
        return nullptr;
    }

    // --- 2. Keyboard grid rows (row 1+) ---
    if (!m_keyboardGrid)
        return nullptr;

    int gridRow = curated ? row - 1 : row;
    if (gridRow < 0)
        return nullptr;

    if (auto* item = m_keyboardGrid->itemAtPosition(gridRow, col))
        return qobject_cast<AACKeyButton*>(item->widget());

    return nullptr;
}

void AACKeyboardScreen::updateUnifiedHighlight()
{
    if (auto* btn = highlightedButton()) {
        btn->setHighlighted(true);
        btn->setFocus();
m_inputController->setPreviousHighlightedButton(btn);
    }
}
void AACKeyboardScreen::keyPressEvent(QKeyEvent* e)
{
    if (!m_inputController) {
        QWidget::keyPressEvent(e);
        return;
    }

    switch (e->key()) {

    case Qt::Key_Left:
        m_inputController->moveLeft();
        return;

    case Qt::Key_Right:
        m_inputController->moveRight();
        return;

    case Qt::Key_Up:
        m_inputController->moveUp();
        return;

    case Qt::Key_Down:
        m_inputController->moveDown();
        return;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        m_inputController->activateCurrent();
        return;

    default:
        QWidget::keyPressEvent(e);
        return;
    }
}

