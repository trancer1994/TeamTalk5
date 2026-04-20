#include "AACScreenBase.h"

AACScreenBase::AACScreenBase(QWidget *parent)
    : QWidget(parent)
{
}

void AACScreenBase::setScreenTitle(const QString &title)
{
    m_title = title;
}

void AACScreenBase::emitInitialTitle()
{
    emit requestTitleChange(m_title);
}
