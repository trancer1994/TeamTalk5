#include "AACTitleBar.h"
#include <QHBoxLayout>

AACTitleBar::AACTitleBar(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(0);

    m_label = new QLabel("-", this);
    m_label->setStyleSheet("font-size: 20px; font-weight: 600;");

    layout->addWidget(m_label);
}

void AACTitleBar::setTitle(const QString &title)
{
    m_label->setText(title);
}
