#include "aac_message_bar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace AAC {

AACMessageBar::AACMessageBar(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);

    m_label = new QLabel(this);
    auto* backspaceBtn = new QPushButton("⌫", this);
    auto* speakBtn = new QPushButton("Speak", this);
    auto* clearBtn = new QPushButton("Clear", this);

    layout->addWidget(m_label);
    layout->addWidget(backspaceBtn);
    layout->addWidget(speakBtn);
    layout->addWidget(clearBtn);

    connect(backspaceBtn, &QPushButton::clicked, this, &AACMessageBar::backspace);
    connect(clearBtn, &QPushButton::clicked, this, &AACMessageBar::clearMessage);
    connect(speakBtn, &QPushButton::clicked, this, [this]() {
        emit messageReady(m_tokens.join(" "));
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

} // namespace AAC
