#pragma once
#include <QWidget>
#include <QLabel>

class AACTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit AACTitleBar(QWidget *parent = nullptr);

public slots:
    void setTitle(const QString &title);

private:
    QLabel *m_label;
};
