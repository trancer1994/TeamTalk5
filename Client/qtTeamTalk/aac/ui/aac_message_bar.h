#pragma once

#include <QWidget>
#include <QStringList>

class QLabel;
class QPushButton;

namespace AAC {

class AACMessageBar : public QWidget
{
    Q_OBJECT

public:
    explicit AACMessageBar(QWidget* parent = nullptr);

public slots:
    void appendSymbol(const QString& label);
    void clearMessage();
    void backspace();

signals:
    void messageReady(const QString& text);

private:
    QLabel* m_label;
    QStringList m_tokens;
};

} // namespace AAC
