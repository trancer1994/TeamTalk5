#pragma once

#include <QPushButton>
#include <QString>

class AACAccessibilityManager;

class AACKeyButton : public QPushButton {
    Q_OBJECT
public:
    AACKeyButton(const QString& text,
                 AACAccessibilityManager* aac,
                 QWidget* parent = nullptr);

    void setHighlighted(bool on);
    bool isHighlighted() const { return m_highlighted; }

signals:
    void keyActivated(const QString& text);
    void hovered(AACKeyButton* self);

protected:
    void enterEvent(QEnterEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void handleClick();

private:
    AACAccessibilityManager* m_aac = nullptr;
    QString m_keyText;
    bool m_highlighted = false;
};
