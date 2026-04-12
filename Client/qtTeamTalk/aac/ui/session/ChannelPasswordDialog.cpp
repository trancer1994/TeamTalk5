#include "ChannelPasswordDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>

ChannelPasswordDialog::ChannelPasswordDialog(QWidget* parent)
    : QDialog(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_edit = new QLineEdit(this);
    m_edit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_edit);

    auto* join = new QPushButton(tr("Join"), this);
    layout->addWidget(join);

    connect(join, &QPushButton::clicked, this, &ChannelPasswordDialog::accept);
}

QString ChannelPasswordDialog::password() const
{
    return m_edit->text();
}
