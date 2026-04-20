#pragma once
#include <QWidget>

class AACScreenBase : public QWidget
{
    Q_OBJECT
public:
    explicit AACScreenBase(QWidget *parent = nullptr);

    void setScreenTitle(const QString &title);
    void emitInitialTitle();

signals:
    void requestTitleChange(const QString &title);

protected:
    QString m_title;
};
