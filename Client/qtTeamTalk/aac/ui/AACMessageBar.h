#pragma once

#include <QWidget>
#include <QStringList>

class QLabel;
class AACAccessibilityManager;

class AACMessageBar : public QWidget
{
    Q_OBJECT
public:
    explicit AACMessageBar(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    // Unified: symbol message goes into AACTextBar
    void symbolMessageReady(const QString& text);

public slots:
    void appendSymbol(const QString& label);
    void clearMessage();
    void backspace();

private:
    QLabel*     m_label = nullptr;
    QStringList m_tokens;
};
