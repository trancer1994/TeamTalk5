#include "NicknameDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>

NicknameDialog::NicknameDialog(QWidget* parent)
    : QDialog(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_edit = new QLineEdit(this);
    layout->addWidget(m_edit);

    auto* save = new QPushButton(tr("Save"), this);
    layout->addWidget(save);

    connect(save, &QPushButton::clicked, this, &NicknameDialog::accept);
}

QString NicknameDialog::nickname() const
{
    return m_edit->text();
}
