#include "AACMainScreen.h"

#include "AACKeyboardScreen.h"
#include "AACTextBar.h"
#include "PredictiveStrip.h"

#include <QHBoxLayout>

AACMainScreen::AACMainScreen(AACAccessibilityManager* aac,
                             QWidget* parent)
    : AACScreenBase(parent)
    , m_aac(aac)
{
    setScreenTitle(tr("AAC"));

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    // Text bar
    m_textBar = new AACTextBar(m_aac, this);
    m_rootLayout->addWidget(m_textBar);

    // Keyboard screen
    m_keyboardScreen = new AACKeyboardScreen(m_aac, this);
    m_rootLayout->addWidget(m_keyboardScreen);

    // Predictive strip
    m_predictiveStrip = new PredictiveStrip(this);
    m_predictiveStrip->setManager(m_aac);
    m_predictiveStrip->setTextBar(m_textBar);

    if (auto* cont = m_keyboardScreen->predictiveStripContainer()) {
        if (!cont->layout()) {
            auto* lay = new QHBoxLayout(cont);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(4);
        }
        cont->layout()->addWidget(m_predictiveStrip);
    }

    // Wiring: text bar → predictive strip
    connect(m_textBar, &AACTextBar::textChanged,
            this, &AACMainScreen::onTextChanged);

    // Wiring: predictive strip → text bar
    connect(m_predictiveStrip, &PredictiveStrip::suggestionChosen,
            this, &AACMainScreen::onSuggestionChosen);

    // Wiring: keyboard → text bar
    connect(m_keyboardScreen, &AACKeyboardScreen::characterTyped,
            this, &AACMainScreen::onCharacterTyped);

    connect(m_keyboardScreen, &AACKeyboardScreen::backspaceRequested,
            this, &AACMainScreen::onBackspace);

    connect(m_keyboardScreen, &AACKeyboardScreen::spaceRequested,
            this, &AACMainScreen::onSpace);

    connect(m_keyboardScreen, &AACKeyboardScreen::clearRequested,
            this, &AACMainScreen::onClear);

    connect(m_keyboardScreen, &AACKeyboardScreen::deleteWordRequested,
            this, &AACMainScreen::onDeleteWord);

    connect(m_keyboardScreen, &AACKeyboardScreen::moveCursorLeft,
            this, &AACMainScreen::onMoveCursorLeft);

    connect(m_keyboardScreen, &AACKeyboardScreen::moveCursorRight,
            this, &AACMainScreen::onMoveCursorRight);

connect(m_textBar, &AACTextBar::cursorMoved,
        this, &AACMainScreen::onCursorMoved);
}

// ------------------------------------------------------------
// AACScreenAdapter
// ------------------------------------------------------------

QList<QWidget*> AACMainScreen::interactiveWidgets() const
{
    return { m_textBar, m_keyboardScreen };
}

QList<QWidget*> AACMainScreen::primaryWidgets() const
{
    return { m_textBar, m_keyboardScreen };
}

QLayout* AACMainScreen::rootLayout() const
{
    return m_rootLayout;
}

// ------------------------------------------------------------
// Wiring
// ------------------------------------------------------------

void AACMainScreen::onTextChanged(const QString& text)
{
    int pos = m_textBar->cursorPosition();
    m_predictiveStrip->setContext(text.left(pos));
}

void AACMainScreen::onSuggestionChosen(const QString& word)
{
    m_textBar->appendWord(word);
}

void AACMainScreen::onCharacterTyped(QChar ch)
{
    m_textBar->insertCharacter(ch);
}

void AACMainScreen::onBackspace()
{
    m_textBar->backspace();
}

void AACMainScreen::onSpace()
{
    m_textBar->insertSpace();
}

void AACMainScreen::onClear()
{
    m_textBar->setText(QString());
}

void AACMainScreen::onDeleteWord()
{
    QString t = m_textBar->text();
    t = t.trimmed();
    int last = t.lastIndexOf(' ');
    if (last >= 0)
        t = t.left(last);
    else
        t.clear();
    m_textBar->setText(t);
}

void AACMainScreen::onMoveCursorLeft()
{
    m_textBar->moveCursorLeft();
}

void AACMainScreen::onMoveCursorRight()
{
    m_textBar->moveCursorRight();
}
void AACMainScreen::onCursorMoved(int pos)
{
    const QString text = m_textBar->text();

    // Prefix up to cursor → prediction context
    const QString prefix = text.left(pos);

    m_predictiveStrip->setContext(prefix);
    m_keyboardScreen->setText(prefix);
}
