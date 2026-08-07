#pragma once

#include <QWidget>
#include <QString>

class AACAccessibilityManager;
class AACInputController;

class AACToggle : public QWidget {
    Q_OBJECT
public:
    explicit AACToggle(const QString& label,
                       AACAccessibilityManager* aac,
                       QWidget* parent = nullptr);

    void setChecked(bool on);
    bool isChecked() const { return m_checked; }

signals:
    void toggled(bool on);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    // AAC infrastructure
    AACAccessibilityManager* m_aac = nullptr;
    AACInputController* m_input = nullptr;

    // Toggle state
    QString m_label;
    bool m_checked = false;

    // AAC highlight state
    bool m_highlighted = false;

    // AAC activation paths
    void activate();          // scanning/dwell/deep‑well activation
    void updateHighlight();   // AAC highlight semantics

    // AAC help mode
    void speakHelp();
};
