#include "AACMainScreen.h"

#include "AACTextBar.h"
#include "AACPredictiveStrip.h"

#include <QHBoxLayout>

AACMainScreen::AACMainScreen(AACAccessibilityManager* aac,
                             QWidget* parent)
    : AACScreenBase(aac, parent)
    , m_aac(aac)
{
    setScreenTitle(tr("AAC"));

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

// Text bar
m_textBar = new AACTextBar(m_aac, this);
m_rootLayout->addWidget(m_textBar);

m_messageBar = new AACMessageBar(...)(m_aac, this);
m_rootLayout->addWidget(m_messageBar);

// Predictive strip directly under text bar
m_predictiveStrip = new PredictiveStrip(this);
m_predictiveStrip->setManager(m_aac);
m_predictiveStrip->setTextBar(m_textBar);
m_rootLayout->addWidget(m_predictiveStrip);

// --- Semantic context label (static, AAC‑safe) ---
m_semanticLabel = new QLabel(this);
m_semanticLabel->setVisible(false);
m_semanticLabel->setStyleSheet(
    "font-size: 16px; color: #555; padding-left: 4px;"
);
m_rootLayout->addWidget(m_semanticLabel);

// --- Cursor-aware semantic breadcrumb ---
m_cursorSemanticLabel = new QLabel(this);
m_cursorSemanticLabel->setVisible(false);
m_cursorSemanticLabel->setStyleSheet(
    "font-size: 14px; color: #666; padding-left: 4px;"
);
m_rootLayout->addWidget(m_cursorSemanticLabel);

auto* buttonRow = new QHBoxLayout();
buttonRow->setContentsMargins(0, 0, 0, 0);
buttonRow->setSpacing(8);

auto* keyboardButton = new AACKeyButton(tr("Keyboard"), m_aac, this);
buttonRow->addWidget(keyboardButton);
buttonRow->addStretch(1);

auto* symbolsButton  = new AACKeyButton(tr("Symbols"), m_aac, this);
buttonRow->addWidget(symbolsButton);
buttonRow->addStretch(1);

auto* sendChannelBtn = new AACKeyButton(tr("Send to Channel"), m_aac, this);
auto* sendPrivateBtn = new AACKeyButton(tr("Send Privately"), m_aac, this);
auto* speakBtn       = new AACKeyButton(tr("Speak"), m_aac, this);

buttonRow->addWidget(sendChannelBtn);
buttonRow->addWidget(sendPrivateBtn);
buttonRow->addWidget(speakBtn);

m_rootLayout->addLayout(buttonRow);

connect(keyboardButton, &AACKeyButton::keyActivated,
        this, [this]() { emit keyboardRequested(); });

connect(symbolsButton, &AACKeyButton::keyActivated,
        this, [this]() { emit symbolGridRequested(); });

connect(sendChannelBtn, &AACKeyButton::keyActivated, this, [this]() {
    m_sendMode = SendMode::Channel;
});

connect(sendPrivateBtn, &AACKeyButton::keyActivated, this, [this]() {
    m_sendMode = SendMode::Private;
});

connect(speakBtn, &AACKeyButton::keyActivated, this, [this]() {
    m_sendMode = SendMode::Speak;
});
// Prediction freeze/unfreeze wiring
{
    connect(this, &AACMainScreen::keyboardRequested, this, [this]() {
        if (m_aac && m_aac->predictionEngine())
            m_aac->predictionEngine()->freezePredictions();
    });

    connect(this, &AACMainScreen::symbolGridRequested, this, [this]() {
        if (m_aac && m_aac->predictionEngine())
            m_aac->predictionEngine()->freezePredictions();
    });

    connect(m_textBar, &AACTextBar::textEdited, this, [this]() {
        if (m_aac && m_aac->predictionEngine())
            m_aac->predictionEngine()->unfreezePredictions();
    });
}
    // Wiring: text bar → predictive strip
    connect(m_textBar, &AACTextBar::textChanged,
            this, &AACMainScreen::onTextChanged);

    // Wiring: predictive strip → text bar
    connect(m_predictiveStrip, &PredictiveStrip::suggestionChosen,
            this, &AACMainScreen::onSuggestionChosen);

connect(m_textBar, &AACTextBar::cursorMoved,
        this, &AACMainScreen::onCursorMoved);

// Symbol message → unified AACTextBar composer
connect(m_messageBar, &AACMessageBar::symbolMessageReady,
        this, [this](const QString& text) {

    QString current = m_textBar->text();
    if (!current.isEmpty())
        current += QLatin1Char(' ');

    current += text;

    m_textBar->setText(current);
    m_textBar->setCursorPosition(current.size());
});
}
QString AACMainScreen::contextualHelp() const {
    return tr(
        "AACMain:\n"
        "Press F4 for Keyboard.\n"
        "Press F5 for Symbol Grid.\n"
        "Press F6 to speak your message.\n"
        "Press F7 to clear your message.\n"
        "Press Escape to go back."
    );
}
// ------------------------------------------------------------
// AACScreenAdapter
// ------------------------------------------------------------

QList<QWidget*> AACMainScreen::interactiveWidgets() const
{
    return { m_textBar };
}

QList<QWidget*> AACMainScreen::primaryWidgets() const
{
    return { m_textBar, };
}

QLayout* AACMainScreen::rootLayout() const
{
    return m_rootLayout;
}

// ------------------------------------------------------------
// Wiring
// ------------------------------------------------------------

void AACMainScreen::onEnterPressed()
{
    const QString text = m_textBar->text();
// Unified send pipeline
    if (m_aac && m_aac->predictionEngine()) {
        const QString trimmed = text.trimmed();
        if (!trimmed.isEmpty())
            m_aac->predictionEngine()->learnUtterance(trimmed.toStdString());
    }
AACMessage msg = buildAACMessageForSend();

switch (m_sendMode) {
case SendMode::Channel:
    emit sendToChannelMessage(msg);
    break;
case SendMode::Private:
    emit sendToUserMessage(msg);
    break;
case SendMode::Speak:
    emit speakAACMessage(msg);
    break;
}
}
void AACMainScreen::onDone()
{
    const QString text = m_textBar->text();
    emit doneRequested();

    if (m_aac && m_aac->predictionEngine()) {
        const QString trimmed = text.trimmed();
        if (!trimmed.isEmpty())
            m_aac->predictionEngine()->learnUtterance(trimmed.toStdString());
    }
}
void AACMainScreen::onTextChanged(const QString& text)
{
    int pos = m_textBar->cursorPosition();
    m_predictiveStrip->setContext(text.left(pos));

if (m_aac && m_aac->predictionEngine()) {
    auto* engine = m_aac->predictionEngine();

QString prefix = text.left(pos);
    QString lastWord = prefix.split(' ').last();

engine->penalizeIgnored(
    std::string(),
        m_predictiveStrip->currentSuggestionList(),
        lastWord.toStdString()
    );
}
}
static QString replaceCurrentWord(const QString &text, int cursorPos, const QString &replacement)
{
    if (cursorPos < 0 || cursorPos > text.size())
        return text;

    int start = cursorPos - 1;
    while (start >= 0 && text.at(start).isLetterOrNumber())
        --start;
    ++start;

    int end = cursorPos;
    while (end < text.size() && text.at(end).isLetterOrNumber())
        ++end;

    QString out = text;
    out.replace(start, end - start, replacement + " ");
    return out;
}
QString AACMainScreen::currentTokenAtCursor() const
{
    const QString text = m_textBar->text();
    int pos = m_textBar->cursorPosition();

    int start = pos - 1;
    while (start >= 0 && text.at(start).isLetterOrNumber())
        --start;
    ++start;

    int end = pos;
    while (end < text.size() && text.at(end).isLetterOrNumber())
        ++end;

    return text.mid(start, end - start);
}

void AACMainScreen::onSuggestionChosen(const QString& word)
{
    const QString text = m_textBar->text();
    const int cursorPos = m_textBar->cursorPosition();

    QString updated = replaceCurrentWord(text, cursorPos, word);

    m_textBar->setText(updated);
    m_textBar->setCursorPosition(updated.size());

if (m_aac && m_aac->predictionEngine()) {
    auto* engine = m_aac->predictionEngine();

    // Previous word before replacement
    QString prev = text.left(cursorPos).split(' ').last();

    engine->reinforceChoice(prev.toStdString(),
                            word.toStdString());

    // Also tell the engine this was a chosen suggestion
    engine->onUserSelected(prev.toStdString(),
                           word.toStdString(),
                           m_predictiveStrip->currentSuggestionList());
}
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

    if (m_accessibility && m_accessibility->predictionEngine())
        m_accessibility->predictionEngine()->setSemanticContext(QString());

    if (m_predictiveStrip)
        m_predictiveStrip->clearSemanticContext();

    // Clear external breadcrumb (if kept)
    if (m_cursorSemanticLabel)
        m_cursorSemanticLabel->setVisible(false);

    // Clear internal breadcrumb
    m_textBar->updateCursorBreadcrumb(QString(), QString());

    emit clearRequested();
}

void AACMainScreen::onDeleteWord()
{
    QString t = m_textBar->text().trimmed();

    // -----------------------------------------
    // Prediction rollback for deleted autocomplete
    // -----------------------------------------
    if (m_aac && m_aac->predictionEngine()) {
        QString deletedWord = t.split(' ').last();
        if (!deletedWord.isEmpty()) {
            m_aac->predictionEngine()->onUserDeletedAutocompleted(
                deletedWord.toStdString()
            );
        }
    }

    // Now actually delete the word
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

    // Token under cursor
    QString token = currentTokenAtCursor();

    // If cursor is in whitespace → clear semantic context + breadcrumb
    if (token.isEmpty()) {
        if (m_aac && m_aac->predictionEngine())
            m_aac->predictionEngine()->setSemanticContext(QString());

        // Clear internal breadcrumb
        m_textBar->updateCursorBreadcrumb(QString(), QString());

        // Hide external breadcrumb (if you keep it)
        if (m_cursorSemanticLabel)
            m_cursorSemanticLabel->setVisible(false);

        return;
    }

    // Compute semantic tag
    QString tag;
    if (m_aac && m_aac->predictionEngine()) {
        m_aac->predictionEngine()->setSemanticContext(token);
        tag = m_aac->predictionEngine()->semanticTagForToken(token);
    }

    // Update internal breadcrumb
    m_textBar->updateCursorBreadcrumb(token, tag);

    // Update external breadcrumb (if you keep it)
    if (m_cursorSemanticLabel) {
        m_cursorSemanticLabel->setText(tr("Editing: \"%1\"").arg(token));
        m_cursorSemanticLabel->setVisible(true);
    }
}
void AACMainScreen::onSemanticContextChanged(const QString& tag)
{
    // 1. Update predictive strip context
    if (m_predictiveStrip) {
        m_predictiveStrip->setSemanticContext(tag);
    }

    // 2. Update phrase suggestions immediately
    if (m_accessibility && m_accessibility->predictionEngine()) {
        auto* eng = m_accessibility->predictionEngine();
        QString prefix = currentTokenAtCursor();
        auto preds = eng->Predict(prefix.toStdString(), 5);

        QStringList out;
        for (auto& p : preds)
            out << QString::fromStdString(p);

        if (m_predictiveStrip)
            m_predictiveStrip->setPredictions(out);
    }

    // 3. Optional: static semantic indicator in UI
    if (!tag.isEmpty()) {
        m_semanticLabel->setText(tr("Context: %1").arg(tag));
        m_semanticLabel->setVisible(true);
    } else {
        m_semanticLabel->setVisible(false);
    }
}
AACMessage AACMainScreen::buildAACMessageForSend() const
{
    AACMessage msg;

    msg.text = m_textBar ? m_textBar->text() : QString();
    msg.isPrivate = (m_sendMode == SendMode::Private);

    if (m_aac) {
        if (m_aac->predictionEngine()) {
            QString token = currentTokenAtCursor();
            QString tag   = m_aac->predictionEngine()->semanticTagForToken(token);
            msg.semanticTag = tag;
            if (!tag.isEmpty())
                msg.tags << tag;
        }

        msg.vocabId = m_aac->activeCategory();
    }

    msg.timestamp = QDateTime::currentDateTime();

    return msg;
}
