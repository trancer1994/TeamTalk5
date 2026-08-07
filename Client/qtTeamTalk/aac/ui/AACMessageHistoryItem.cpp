#include "AAC.h"
#include "AACMessageHistoryItem.h"
#include <QClipboard>
#include <QApplication>

AACMessageHistoryItem::AACMessageHistoryItem(const AACMessageHistoryEvent& ev,
                                             QWidget* parent)
    : QWidget(parent)
    , m_event(ev)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(4);

    // --- Top row: text + timestamp ---
    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(6);

    m_textLabel = new QLabel(ev.text, this);
    m_textLabel->setWordWrap(true);

    m_timestampLabel = new QLabel(ev.timestamp.toString("yyyy-MM-dd hh:mm:ss"), this);

    topRow->addWidget(m_textLabel, 1);
    topRow->addWidget(m_timestampLabel, 0);

    // --- Semantic tag row ---
    m_semanticLabel = new QLabel(
        ev.semanticTag.isEmpty()
            ? tr("Semantic: (none)")
            : tr("Semantic: %1").arg(ev.semanticTag),
        this);

    // --- Buttons row ---
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(6);

    m_replayBtn = new AACKeyButton(tr("Replay"), this);
    m_speakBtn  = new AACKeyButton(tr("Speak"), this);
    m_copyBtn   = new AACKeyButton(tr("Copy"), this);
    m_deleteBtn = new AACKeyButton(tr("Delete"), this);

    btnRow->addWidget(m_replayBtn);
    btnRow->addWidget(m_speakBtn);
    btnRow->addWidget(m_copyBtn);
    btnRow->addWidget(m_deleteBtn);

    // --- Add rows to root ---
    root->addLayout(topRow);
    root->addWidget(m_semanticLabel);
    root->addLayout(btnRow);

    // --- AAC-native speech feedback ---
AAC::setElementHelp(m_textLabel,
    tr("Message text: %1").arg(ev.text));

AAC::setElementHelp(m_timestampLabel,
    tr("Sent at %1").arg(ev.timestamp.toString()));

AAC::setElementHelp(m_semanticLabel,
    ev.semanticTag.isEmpty()
        ? tr("Semantic tag: (none)")
        : tr("Semantic tag: %1").arg(ev.semanticTag));

AAC::setElementHelp(m_replayBtn, tr("Replay this message"));
AAC::setElementHelp(m_speakBtn,  tr("Speak this message aloud"));
AAC::setElementHelp(m_copyBtn,   tr("Copy this message to clipboard"));
AAC::setElementHelp(m_deleteBtn, tr("Delete this message from history"));

    // --- Wiring ---
    connect(m_replayBtn, &AACKeyButton::keyActivated, this, [this] {
        emit requestReplay(m_event);
    });

    connect(m_speakBtn, &AACKeyButton::keyActivated, this, [this] {
        emit requestSpeak(m_event);
    });

    connect(m_copyBtn, &AACKeyButton::keyActivated, this, [this] {
        QClipboard* cb = QApplication::clipboard();
        cb->setText(m_event.text);
        emit requestCopy(m_event);
    });

    connect(m_deleteBtn, &AACKeyButton::keyActivated, this, [this] {
        emit requestDelete(m_event);
    });
}
