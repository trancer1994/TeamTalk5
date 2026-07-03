#include "ChannelPasswordScreen.h"
#include "AACKeyButton.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

ChannelPasswordScreen::ChannelPasswordScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    auto* main = new QVBoxLayout(this);
    auto* label = new QLabel(tr("Channel password"), this);
    main->addWidget(label);

    m_edit = new QLineEdit(this);
    m_edit->setEchoMode(QLineEdit::Password);
    main->addWidget(m_edit);

    auto* row = new QHBoxLayout();
    m_joinBtn = new AACKeyButton(aac, tr("Join"), this);
    m_cancelBtn = new AACKeyButton(aac, tr("Cancel"), this);
    row->addWidget(m_joinBtn);
    row->addWidget(m_cancelBtn);
    main->addLayout(row);

    connect(m_joinBtn, &AACKeyButton::activated, this, [this]() {
        emit passwordEntered(m_edit->text());
    });
    connect(m_cancelBtn, &AACKeyButton::activated, this, [this]() {
        emit cancelled();
    });

    setLayout(main);
}
QString ChannelPasswordScreen::contextualHelp() const
{
    return tr("ChannelPassword. "
               "Type the password and press Enter. "
               "Press Escape to go back.");
}