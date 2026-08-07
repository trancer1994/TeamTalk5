#pragma once

#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QStringList>

class AACAccessibilityManager;

class AACKeyButton : public QPushButton {
    Q_OBJECT
public:
    AACKeyButton(const QString& text,
                 AACAccessibilityManager* aac,
                 QWidget* parent = nullptr);

    void setHighlighted(bool on);
    bool isHighlighted() const { return m_highlighted; }

    void setVariants(const QStringList& v) { m_variants = v; }

void setSemanticPulse(bool on) { m_semanticPulse = on; update(); }
    void setDeepWell(bool enabled);
    bool isDeepWell() const { return m_deepWell; }

    void setDwellProgress(float p);

void setSemanticHighlighted(bool on, const QColor& color) {
    m_semanticHighlighted = on;
    m_semanticColor = color;
    update();
}
    bool isSemanticHighlighted() const { return m_semanticHighlighted; }

signals:
    void keyActivated(const QString& text);
    void variantChosen(const QString& text);
    void hovered(AACKeyButton* self);
    void longPressed();
    void deepWellActivated(const QString& text);

protected:
    void enterEvent(QEnterEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    AACAccessibilityManager* m_aac = nullptr;
    QString m_keyText;
    bool m_highlighted = false;
bool m_semanticHighlighted = false;
bool m_semanticPulse = false;

    QColor m_semanticColor = QColor(0, 120, 215);  // ⭐ store role colour

    // Long‑press support
    QStringList m_variants;
    QTimer *m_longPressTimer = nullptr;
    bool m_isLongPressActive = false;

    bool m_deepWell = false;

    float m_dwellProgress = 0.0f;
};
