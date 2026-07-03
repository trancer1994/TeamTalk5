#include "AACCommunicationMethodScreen.h"
#include "AACKeyButton.h"
#include <QVBoxLayout>

AACCommunicationMethodScreen::AACCommunicationMethodScreen(AACAccessibilityManager* aac, QWidget* parent)
    : AACScreenBase(aac, parent)
{
    setScreenTitle(tr("How do you communicate?"));

    auto* layout = new QVBoxLayout(this);

    auto add = [&](const QString& text, CommMethod m) {
        auto* btn = new AACKeyButton(aac, text, this);
        layout->addWidget(btn);
        connect(btn, &AACKeyButton::activated, this, [this, m]() {
            emit communicationMethodChosen(m);
        });
    };

    add(tr("Touch / Click"), CommMethod::TouchClick);
    add(tr("Gaze + Dwell"), CommMethod::GazeDwell);
    add(tr("Switch Scanning"), CommMethod::SwitchScanning);

    setLayout(layout);
}
