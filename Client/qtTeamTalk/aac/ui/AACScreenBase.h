#pragma once
#include <QWidget>
#include <QKeyEvent>
#include <QString>

class AACAccessibilityManager;

class AACScreenBase : public QWidget
{
    Q_OBJECT
public:
    explicit AACScreenBase(AACAccessibilityManager* aac,
                           QWidget* parent = nullptr);

    virtual ~AACScreenBase() = default;

    // Title handling
    void setScreenTitle(const QString& title);
    void emitInitialTitle();

    // Help system (override in derived screens)
    virtual QString screenLevelHelp() const { return QString(); }
    virtual QString contextualHelp() const { return tr("Help not available for this screen."); }
    virtual QString contextualHelpForElement(const QString& id) const {
        Q_UNUSED(id);
        return QString();
    }

signals:
    void requestTitleChange(const QString& title);

protected:
    // Global key handling (F‑keys, Escape, scanning/dwell gating)
    void keyPressEvent(QKeyEvent* e) override;

    AACAccessibilityManager* m_aac = nullptr;
    QString m_title;
};
