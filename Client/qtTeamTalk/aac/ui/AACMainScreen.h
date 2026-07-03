#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "AACScreenBase.h"
#include "aac/AACFramework.h"
#include "aac/models/AACMessage.h"

class AACTextBar;
class PredictiveStrip;

class AACMainScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit AACMainScreen(AACAccessibilityManager* aac,
                           QWidget* parent = nullptr);

    // AACScreenAdapter
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;

public slots:
    void onSemanticContextChanged(const QString& tag);

signals:
void sendToChannelMessage(const AACMessage& msg);
void sendToUserMessage(const AACMessage& msg);
void speakAACMessage(const AACMessage& msg);
    void clearRequested();
    void doneRequested();
    void keyboardRequested();
    void symbolGridRequested();

private slots:
    void onEnterPressed();
    void onDone();
    void onTextChanged(const QString& text);
    void onSuggestionChosen(const QString& word);
    void onCursorMoved(int pos);

    void onCharacterTyped(QChar ch);
    void onBackspace();
    void onSpace();
    void onClear();
    void onDeleteWord();
    void onMoveCursorLeft();
    void onMoveCursorRight();

private:
AACMessage buildAACMessageForSend() const;
enum class SendMode { Channel, Private, Speak };
SendMode m_sendMode = SendMode::Channel;
    AACAccessibilityManager* m_aac = nullptr;

QLabel* m_semanticLabel = nullptr;
QLabel* m_cursorSemanticLabel = nullptr;
    QVBoxLayout* m_rootLayout = nullptr;

    AACTextBar*        m_textBar        = nullptr;
    PredictiveStrip*   m_predictiveStrip = nullptr;
QString currentTokenAtCursor() const;
};
