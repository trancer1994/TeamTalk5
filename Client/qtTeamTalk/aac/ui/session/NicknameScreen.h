#pragma once
#include "AACScreenBase.h"

class QLineEdit;
class AACKeyButton;

class NicknameScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit NicknameScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void nicknameChosen(const QString& name);
    void cancelled();

private:
    QLineEdit*    m_edit = nullptr;
    AACKeyButton* m_okBtn = nullptr;
    AACKeyButton* m_cancelBtn = nullptr;
};
