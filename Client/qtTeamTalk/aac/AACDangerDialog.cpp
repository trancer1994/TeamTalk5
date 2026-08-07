#include "AACDangerDialog.h"
#include "AACKeyButton.h"
#include <QVBoxLayout>
#include <QLabel>

AACDangerDialog::AACDangerDialog(const QString& title,
                                 const QString& message,
                                 AACAccessibilityManager* mgr,
                                 QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    setProperty("aacDangerDialog", true);

    auto* layout = new QVBoxLayout(this);
    auto* label  = new QLabel(message, this);
    label->setWordWrap(true);
    layout->addWidget(label);

    auto* btnYes = new AACKeyButton(tr("Yes"), mgr, this);
    auto* btnNo  = new AACKeyButton(tr("No"), mgr, this);

    btnYes->setProperty("aacPrimary", true);
    btnNo->setProperty("aacSecondary", true);

    connect(btnYes, &AACKeyButton::keyActivated,
            this, [this]() { accept(); });
    connect(btnNo, &AACKeyButton::keyActivated,
            this, [this]() { reject(); });

    layout->addWidget(btnYes);
    layout->addWidget(btnNo);
}

bool AACDangerDialog::confirm(QWidget* parent,
                              AACAccessibilityManager* mgr,
                              const QString& title,
                              const QString& message)
{
    AACDangerDialog dlg(title, message, mgr, parent);
    return dlg.exec() == QDialog::Accepted;
}
