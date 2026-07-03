#pragma once
#include <QWidget>

class AACScreenBase : public QWidget
{
    Q_OBJECT
public:
    explicit AACScreenBase(QWidget *parent = nullptr);

    void setScreenTitle(const QString &title);
    void emitInitialTitle();
virtual QString screenLevelHelp() const { return QString(); }
virtual QString contextualHelpForElement(const QString& id) const {
    Q_UNUSED(id);
    return QString();
}

signals:
    void requestTitleChange(const QString &title);

protected:
    QString m_title;
};
