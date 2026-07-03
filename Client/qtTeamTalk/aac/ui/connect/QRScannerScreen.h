#pragma once
#include "AACScreenBase.h"

class AACKeyButton;
class QLabel;

class QRScannerScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit QRScannerScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void qrPayloadDetected(const QString& payload);
    void backRequested();

protected:
    QString contextualHelp() const override;

private slots:
    void onFakeScan(); // placeholder until camera API is added

private:
    QLabel* m_previewLabel = nullptr;
    AACKeyButton* m_scanBtn = nullptr;
    AACKeyButton* m_backBtn = nullptr;
};
