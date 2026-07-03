#include "AACTypingMethodScreen.h"
#include "AACButton.h"
#include <QVBoxLayout>
#include <QLabel>

AACTypingMethodScreen::AACTypingMethodScreen(AACAccessibilityManager* aac,
                                             QWidget* parent)
    : AACScreenBase(aac, parent)
{
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(40, 40, 40, 40);
    lay->setSpacing(30);

    auto* title = new QLabel(tr("How do you type?"), this);
    title->setWordWrap(true);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 32px; font-weight: bold;");
    lay->addWidget(title);

    // Keyboard only
    auto* btnKeyboard = new AACButton(aac, this);
    btnKeyboard->setText(tr("Keyboard Only"));
    connect(btnKeyboard, &QPushButton::clicked, this, [this] {
        emit typingMethodChosen(TypingMethod::KeyboardOnly);
    });
    lay->addWidget(btnKeyboard);

    // Symbol grid only
    auto* btnSymbols = new AACButton(aac, this);
    btnSymbols->setText(tr("Symbol Grid Only"));
    connect(btnSymbols, &QPushButton::clicked, this, [this] {
        emit typingMethodChosen(TypingMethod::SymbolGrid);
    });
    lay->addWidget(btnSymbols);

    // Both
    auto* btnBoth = new AACButton(aac, this);
    btnBoth->setText(tr("Keyboard + Symbol Grid"));
    connect(btnBoth, &QPushButton::clicked, this, [this] {
        emit typingMethodChosen(TypingMethod::Both);
    });
    lay->addWidget(btnBoth);

    setRootLayout(lay);
}
