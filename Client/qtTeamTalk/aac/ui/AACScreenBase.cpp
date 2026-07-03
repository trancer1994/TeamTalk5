#include "AACScreenBase.h"

AACScreenBase::AACScreenBase(QWidget *parent)
    : QWidget(parent)
{
}
QString AACScreenBase::contextualHelp() const 
{
                return tr("Help not available for this screen.");
}

void AACScreenBase::setScreenTitle(const QString &title)
{
    m_title = title;
}

void AACScreenBase::emitInitialTitle()
{
    emit requestTitleChange(m_title);
}
void AACScreenBase::keyPressEvent(QKeyEvent* e)
{
    // Global Help (F1)
    if (e->key() == Qt::Key_F1) {
        if (m_aac && m_aac->earcons())
            m_aac->earcons()->help();

        if (m_aac && m_aac->speechEngine())
            m_aac->speechEngine()->speak(contextualHelp());

        return;
    }

    QWidget::keyPressEvent(e);
}
