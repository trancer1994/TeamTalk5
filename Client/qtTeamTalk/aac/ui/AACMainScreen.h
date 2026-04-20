#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "AACScreenBase.h"
#include "aac/AACFramework.h"

class AACKeyboardScreen;
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

signals:
    void textCommitted(const QString& text);

private slots:
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
    AACAccessibilityManager* m_aac = nullptr;

    QVBoxLayout* m_rootLayout = nullptr;

    AACTextBar*        m_textBar        = nullptr;
    AACKeyboardScreen* m_keyboardScreen = nullptr;
    PredictiveStrip*   m_predictiveStrip = nullptr;
};
