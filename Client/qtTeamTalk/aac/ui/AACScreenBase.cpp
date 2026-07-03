#include "AACScreenBase.h"
#include "AACAccessibilityManager.h"
#include <QKeyEvent>

AACScreenBase::AACScreenBase(AACAccessibilityManager* aac,
                             QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
{
    setObjectName("AACScreenBase");
}

void AACScreenBase::setScreenTitle(const QString& title)
{
    m_title = title;
    setAccessibleName(title);
}

void AACScreenBase::emitInitialTitle()
{
    emit requestTitleChange(m_title);
}

void AACScreenBase::keyPressEvent(QKeyEvent* e)
{
    if (!m_aac) {
        QWidget::keyPressEvent(e);
        return;
    }

    const auto modes = m_aac->modes();

    // --- F1: contextual help ---
    if (e->key() == Qt::Key_F1) {
        if (modes.scanning || modes.dwell)
            return;

        if (m_aac->earcons())
            m_aac->earcons()->help();

        if (m_aac->speechEngine())
            m_aac->speechEngine()->speak(contextualHelp());

        return;
    }

    // --- F12: screen-level help ---
    if (e->key() == Qt::Key_F12) {
        if (modes.scanning || modes.dwell)
            return;

        if (m_aac->earcons())
            m_aac->earcons()->help();

        if (m_aac->speechEngine())
            m_aac->speechEngine()->speak(screenLevelHelp());

        return;
    }

    // --- Escape: go back ---
    if (e->key() == Qt::Key_Escape) {
        if (modes.scanning || modes.dwell)
            return;

        if (m_aac->earcons())
            m_aac->earcons()->back();

        emit requestTitleChange(QString());
        return;
    }

    QWidget::keyPressEvent(e);
}
