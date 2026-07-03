#pragma once
#include "AACScreenBase.h"
#include "AACKeyButton.h"

class ConnectionHubScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit ConnectionHubScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void connectManualRequested();
    void findServersRequested();
    void publicServersRequested();
    void joinCodeRequested();
    void qrScanRequested();
    void reconnectRequested();
    void backRequested();

protected:
    QString contextualHelp() const override;
};
