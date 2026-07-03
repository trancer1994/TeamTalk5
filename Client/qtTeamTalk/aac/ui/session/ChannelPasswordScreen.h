#pragma once
#include "AACScreenBase.h"

class QLineEdit;
class AACKeyButton;

class ChannelPasswordScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit ChannelPasswordScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void passwordEntered(const QString& pw);
    void cancelled();

private:
    QLineEdit*    m_edit = nullptr;
    AACKeyButton* m_joinBtn = nullptr;
    AACKeyButton* m_cancelBtn = nullptr;
};
