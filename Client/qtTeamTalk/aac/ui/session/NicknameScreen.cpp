#include "NicknameScreen.h"
#include "AACKeyButton.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

NicknameScreen::NicknameScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    auto* main = new QVBoxLayout(this);
    auto* label = new QLabel(tr("Nickname"), this);
    main->addWidget(label);

    m_edit = new QLineEdit(this);
    main->addWidget(m_edit);

    auto* row = new QHBoxLayout();
    m_okBtn = new AACKeyButton(aac, tr("Continue"), this);
    m_cancelBtn = new AACKeyButton(aac, tr("Cancel"), this);
    row->addWidget(m_okBtn);
    row->addWidget(m_cancelBtn);
    main->addLayout(row);

    connect(m_okBtn, &AACKeyButton::activated, this, [this]() {
        emit nicknameChosen(m_edit->text());
    });
    connect(m_cancelBtn, &AACKeyButton::activated, this, [this]() {
        emit cancelled();
    });

    setLayout(main);
}
QString NicknameScreen::contextualHelp() const
{
    return tr("Nickname. "
               "Type your nickname and press Enter. "
               "Press Escape to go back.");
}

