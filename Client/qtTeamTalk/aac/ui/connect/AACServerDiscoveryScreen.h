#pragma once

#include "AACScreenBase.h"
#include "aac/ui/AACKeyButton.h"

class AACServerDiscoveryScreen : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACServerDiscoveryScreen(AACAccessibilityManager* aac,
                                      QWidget* parent = nullptr);

    QString contextualHelp() const override;

signals:
    void lanRequested();
    void publicServersRequested();
    void recentServersRequested();
    void joinCodeRequested();
    void qrScanRequested();
    void deepLinkRequested();   // NEW
    void backRequested();

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    AACKeyButton* m_lanBtn = nullptr;
    AACKeyButton* m_publicBtn = nullptr;
    AACKeyButton* m_recentBtn = nullptr;
    AACKeyButton* m_joinCodeBtn = nullptr;
    AACKeyButton* m_qrBtn = nullptr;
    AACKeyButton* m_deepLinkBtn = nullptr;   // NEW
    AACKeyButton* m_backBtn = nullptr;
};
