#include "AACMessageBar.h"
#include "AACKeyButton.h"

#include <QHBoxLayout>
#include <QLabel>

AACMessageBar::AACMessageBar(AACAccessibilityManager* aac, QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);

    m_label = new QLabel(this);
    m_label->setWordWrap(false);
    m_label->setMinimumHeight(40);
    layout->addWidget(m_label, 1);

    auto* backspaceBtn = new AACKeyButton(aac, tr("⌫"), this);
    auto* speakBtn     = new AACKeyButton(aac, tr("Speak"), this);
    auto* clearBtn     = new AACKeyButton(aac, tr("Clear"), this);

    layout->addWidget(backspaceBtn);
    layout->addWidget(speakBtn);
    layout->addWidget(clearBtn);

    connect(backspaceBtn, &AACKeyButton::activated,
            this, &AACMessageBar::backspace);

    connect(clearBtn, &AACKeyButton::activated,
            this, &AACMessageBar::clearMessage);

    connect(speakBtn, &AACKeyButton::activated, this, [this]() {
        const QString text = m_tokens.join(" ");
        if (!text.isEmpty())
            emit symbolMessageReady(text);
    });

    setLayout(layout);
}

void AACMessageBar::appendSymbol(const QString& label)
{
    m_tokens.append(label);
    m_label->setText(m_tokens.join(" "));
}

void AACMessageBar::clearMessage()
{
    m_tokens.clear();
    m_label->clear();
}

void AACMessageBar::backspace()
{
    if (!m_tokens.isEmpty()) {
        m_tokens.removeLast();
        m_label->setText(m_tokens.join(" "));
    }
}
