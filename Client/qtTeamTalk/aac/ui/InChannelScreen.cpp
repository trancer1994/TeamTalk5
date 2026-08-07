#include "InChannelScreen.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>

#include "AACMainScreen.h"

// ------------------------------------------------------------
// UserRowWidget: one row in the user list (visual only)
// ------------------------------------------------------------
class UserRowWidget : public QWidget
{
public:
    UserRowWidget(const QString& username,
                  int userId,
                  AACAccessibilityManager* aac,
                  BackendAdapter* backend,
                  QWidget* parent = nullptr)
        : QWidget(parent)
        , m_userId(userId)
        , m_aac(aac)
        , m_backend(backend)
    {
setFocusPolicy(Qt::NoFocus);
setAccessibleName(username);
setAccessibleDescription(tr("User %1 with volume controls and speaking indicator").arg(username));
setAccessibleRole(QAccessible::ListItem);

        QHBoxLayout* lay = new QHBoxLayout(this);
        lay->setContentsMargins(4, 2, 4, 2);
        lay->setSpacing(8);

        m_nameLabel = new QLabel(username, this);
m_nameLabel->setFocusPolicy(Qt::NoFocus);
        m_micLabel  = new QLabel(this);
m_micLabel->setFocusPolicy(Qt::NoFocus);
        m_micLabel->setPixmap(QPixmap(":/icons/mic.png").scaled(24, 24));
        m_micLabel->setVisible(false);

        // AAC‑native volume buttons
        m_quieterBtn = new AACKeyButton(tr("−"), m_aac, this);
        m_louderBtn  = new AACKeyButton(tr("+"), m_aac, this);
        m_resetBtn   = new AACKeyButton(tr("Normal"), m_aac, this);

        m_quieterBtn->setMinimumWidth(60);
m_quieterBtn->setAccessibleName(tr("Decrease volume"));
        m_louderBtn->setMinimumWidth(60);
m_louderBtn->setAccessibleName(tr("Increase volume"));
        m_resetBtn->setMinimumWidth(90);
m_resetBtn->setAccessibleName(tr("Reset volume to normal"));

        connect(m_quieterBtn, &AACKeyButton::keyActivated,
                this, [this](const QString&) { adjustVolume(-10); });

        connect(m_louderBtn, &AACKeyButton::keyActivated,
                this, [this](const QString&) { adjustVolume(+10); });

        connect(m_resetBtn, &AACKeyButton::keyActivated,
                this, [this](const QString&) { resetVolume(); });

        lay->addWidget(m_nameLabel);
        lay->addStretch(1);
        lay->addWidget(m_micLabel);
        lay->addWidget(m_quieterBtn);
        lay->addWidget(m_louderBtn);
        lay->addWidget(m_resetBtn);
    }
QString username() const { return m_nameLabel->text(); }

    void setSpeaking(bool speaking)
    {
        m_micLabel->setVisible(speaking);
    }

private:
    void adjustVolume(int delta)
    {
        if (!m_aac || !m_backend)
            return;

        int percent = 100;
        if (m_aac->accessibilityManager()->hasUserVolume(m_userId))
            percent = m_aac->accessibilityManager()->userVolume(m_userId);

        percent = qBound(0, percent + delta, 100);

        // Save
        m_aac->accessibilityManager()->setUserVolume(m_userId, percent);
        m_aac->accessibilityManager()->persist();

        // Apply
        int vol = refVolume(percent);
        TT_SetUserVolume(m_backend->ttInstance(),
                         m_userId,
                         STREAMTYPE_VOICE,
                         vol);
    }

    void resetVolume()
    {
        if (!m_aac || !m_backend)
            return;

        int percent = 100;

        m_aac->accessibilityManager()->setUserVolume(m_userId, percent);
        m_aac->accessibilityManager()->persist();

        int vol = refVolume(percent);
        TT_SetUserVolume(m_backend->ttInstance(),
                         m_userId,
                         STREAMTYPE_VOICE,
                         vol);
    }

    int m_userId;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_micLabel  = nullptr;
    AACKeyButton* m_quieterBtn = nullptr;
    AACKeyButton* m_louderBtn  = nullptr;
    AACKeyButton* m_resetBtn   = nullptr;
    AACAccessibilityManager* m_aac = nullptr;
    BackendAdapter* m_backend = nullptr;
};

void updateUser(const QString& username,
                int userId,
                AACAccessibilityManager* aac,
                BackendAdapter* backend)
{
    m_nameLabel->setText(username);
    m_userId = userId;
    m_aac = aac;
    m_backend = backend;

    setAccessibleName(username);
    setAccessibleDescription(
        tr("User %1 with volume controls and speaking indicator").arg(username)
    );
}
// ------------------------------------------------------------
// UserListWidget: static list of users in the channel (visual only)
// ------------------------------------------------------------
class UserListWidget : public QWidget
{
public:
    explicit UserListWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(8);
setAccessibleName(tr("User list"));
setAccessibleDescription(tr("People currently in the channel"));
setAccessibleRole(QAccessible::List);
setFocusPolicy(Qt::NoFocus);
    }
void UserListWidget::setUsers(const QList<QString>& usernames,
                              const QList<int>& userIds,
                              AACAccessibilityManager* aac,
                              BackendAdapter* backend)
{
    const int count = qMin(usernames.size(), userIds.size());

    // Resize rows if needed
    while (m_rows.size() < count) {
        auto* row = new UserRowWidget("", 0, aac, backend, this);
        m_rows.append(row);
        m_layout->addWidget(row);
    }

    // Hide extra rows
    for (int i = count; i < m_rows.size(); ++i)
        m_rows[i]->setVisible(false);

    // Update visible rows
    for (int i = 0; i < count; ++i) {
        m_rows[i]->setVisible(true);
        m_rows[i]->updateUser(usernames[i], userIds[i], aac, backend);
    }
}

    void setUserSpeaking(const QString& username, bool speaking)
    {
        for (auto* row : m_rows) {
            if (row->username() == username) {
                row->setSpeaking(speaking);
row->setFocusPolicy(Qt::NoFocus);
                return;
            }
        }
    }
void InChannelScreen::keyPressEvent(QKeyEvent* e)
{
// ⭐ F2: Speak current recipient
if (e->key() == Qt::Key_F2) {
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        QString rec = m_sendToChannel
            ? tr("Sending to channel")
            : tr("Sending to %1").arg(m_currentUserRecipient);
        m_aac->speechEngine()->speakNotification(rec);
    }
    return;
}
// ⭐ Shift+F3: Speak presence summary (AAC‑native, avoids global conflict)
if (e->key() == Qt::Key_F3 && (e->modifiers() & Qt::ShiftModifier)) {
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode)
        m_aac->speechEngine()->speakNotification(m_presenceLabel->text());
    return;
}
    // ⭐ F4: Speak channel status (speaking + transmit + presence)
    if (e->key() == Qt::Key_F4) {
        if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
            QString status = QString("%1. %2. %3.")
                .arg(m_speakingLabel->text())
                .arg(m_transmitStatusLabel->text())
                .arg(m_presenceLabel->text());
            m_aac->speechEngine()->speakNotification(status);
        }
        return;
    }

// ⭐ Shift+F4: Speak active speakers
if (e->key() == Qt::Key_F4 && (e->modifiers() & Qt::ShiftModifier)) {
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        if (m_activeSpeakers.isEmpty()) {
            m_aac->speechEngine()->speakNotification(tr("No one is speaking"));
        } else {
            QStringList names;
            for (int id : m_activeSpeakers)
                names << m_backend->usernameForId(id);
            m_aac->speechEngine()->speakNotification(
                tr("Active speakers: %1").arg(names.join(", "))
            );
        }
    }
    return;
}
    // ⭐ F5: Speak last message without entering the log
    if (e->key() == Qt::Key_F5) {
        if (m_messageLog && m_messageLog->count() > 0 &&
            m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
            auto* item = m_messageLog->item(m_messageLog->count() - 1);
            m_aac->speechEngine()->speakNotification(item->text());
        }
        return;
    }

    // ⭐ F6: Jump to message log (blind‑friendly, AAC‑safe)
    if (e->key() == Qt::Key_F6) {
        if (m_messageLog) {
            m_messageLog->setFocus();

            // Announce BEFORE resetting
            if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
                if (m_newMessageCount > 0) {
                    m_aac->speechEngine()->speakNotification(
                        tr("Message log: %1 new messages").arg(m_newMessageCount)
                    );
                } else {
                    m_aac->speechEngine()->speakNotification(tr("Message log"));
                }
            }

            // Reset AFTER announcing
            m_newMessageCount = 0;
            m_newMessageIndicator->clear();
        }
        return;
    }
// ⭐ Shift+F6: Speak unread count
if (e->key() == Qt::Key_F6 && (e->modifiers() & Qt::ShiftModifier)) {
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        QString msg = (m_newMessageCount > 0)
            ? tr("%1 unread messages").arg(m_newMessageCount)
            : tr("No unread messages");
        m_aac->speechEngine()->speakNotification(msg);
    }
    return;
}
// ⭐ F7: Toggle AACMessageHistoryViewer (AAC‑modal screen)
if (e->key() == Qt::Key_F7) {
    if (!m_historyContainer || !m_historyViewer)
        return;

    if (m_historyContainer->isVisible()) {
        // Close
        m_historyContainer->setVisible(false);
        this->setFocus();

        if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode)
            m_aac->speechEngine()->speakNotification(tr("Closed history"));
    } else {
        // Open
        m_historyContainer->setVisible(true);
        m_historyContainer->raise();
        m_historyViewer->setFocus();

        if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode)
            m_aac->speechEngine()->speakNotification(tr("Conversation history"));

        m_historyViewer->jumpToLastEvent();
    }
    return;
}
// ⭐ Shift+F10: Speak transmit status
if (e->key() == Qt::Key_F10 && (e->modifiers() & Qt::ShiftModifier)) {
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        m_aac->speechEngine()->speakNotification(m_transmitStatusLabel->text());
    }
    return;
}
// ⭐ Shift+F11: Speak channel name
if (e->key() == Qt::Key_F11 && (e->modifiers() & Qt::ShiftModifier)) {
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        m_aac->speechEngine()->speakNotification(m_channelLabel->text());
    }
    return;
}
    // Default handling
    QWidget::keyPressEvent(e);
}
private:
    QVBoxLayout* m_layout = nullptr;
    QList<UserRowWidget*> m_rows;
};

#include "InChannelScreen.moc"

// ------------------------------------------------------------
// InChannelScreen
// ------------------------------------------------------------
InChannelScreen::InChannelScreen(AACAccessibilityManager* aac,
                                 BackendAdapter* backend,
                                 QWidget* parent)
    : AACScreenBase(parent)
    , m_aac(aac)
    , m_backend(backend)
{
    setScreenTitle("Channel");

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(12);

    m_channelLabel   = new QLabel(tr("Channel"), this);
m_channelLabel->setFocusPolicy(Qt::NoFocus);
m_channelLabel->setAccessibleName(tr("Channel name"));
m_channelLabel->setAccessibleRole(QAccessible::StaticText);
    m_eventLabel     = new QLabel(this);
m_eventLabel->setAccessibleRole(QAccessible::StaticText);
    m_speakingLabel  = new QLabel(tr("Channel quiet"), this);
m_speakingLabel->setFocusPolicy(Qt::NoFocus);
m_speakingLabel->setAccessibleName(tr("Speaking status"));
m_speakingLabel->setAccessibleRole(QAccessible::StaticText);
    m_presenceLabel  = new QLabel(tr("Participants: no one here"), this);
m_presenceLabel->setAccessibleRole(QAccessible::StaticText);
    m_transmitModeLabel = new QLabel(tr("Transmit mode: Toggle transmit"), this);
m_transmitModeLabel->setFocusPolicy(Qt::NoFocus);
m_transmitModeLabel->setAccessibleName(tr("Transmit mode"));
m_transmitModeLabel->setAccessibleDescription(
    tr("Tap to toggle, continuous, voice activation, or auto silence")
);
m_transmitModeLabel->setAccessibleRole(QAccessible::StaticText);
m_transmitModeLabel->setObjectName("transmitModeLabel");
m_transmitStatusLabel = new QLabel(tr("Not transmitting"), this);
m_transmitStatusLabel->setAccessibleName(tr("Transmit status"));
m_transmitStatusLabel->setAccessibleRole(QAccessible::StaticText);
m_transmitStatusLabel->setFocusPolicy(Qt::NoFocus);
m_transmitStatusLabel->setVisible(true);
m_transmitStatusLabel->setObjectName("transmitStatusLabel");

m_newMessageIndicator = new QLabel(this);
m_newMessageIndicator->setObjectName("newMessageIndicator");
m_newMessageIndicator->setText(QString());
m_newMessageIndicator->setAccessibleName(tr("New message indicator"));
m_newMessageIndicator->setAccessibleRole(QAccessible::StaticText);
m_newMessageIndicator->setFocusPolicy(Qt::NoFocus);
            m_newMessageIndicator->clear();
        });

    m_presenceLabel->setFocusPolicy(Qt::NoFocus);
    m_presenceLabel->setAccessibleName(tr("People here"));

    m_userList = new UserListWidget(this);
m_userList->setObjectName("userList");

// NEW: instantiate AACMainScreen
m_aacMain = new AACMainScreen(m_aac, this);

// ------------------------------------------------------------
// History Viewer (AAC-modal screen, visually prominent)
// ------------------------------------------------------------
m_historyContainer = new QWidget(this);
m_historyContainer->setVisible(false);
m_historyContainer->setAccessibleName(tr("Conversation history screen"));
m_historyContainer->setAccessibleRole(QAccessible::Pane);
m_historyContainer->setStyleSheet("background-color: #202020;"); // visually distinct

QVBoxLayout* histLay = new QVBoxLayout(m_historyContainer);
histLay->setContentsMargins(12, 12, 12, 12);
histLay->setSpacing(8);

// Header
QLabel* histHeader = new QLabel(tr("Conversation history"), m_historyContainer);
histHeader->setAccessibleName(tr("Conversation history header"));
histHeader->setAccessibleRole(QAccessible::Heading);
histHeader->setFocusPolicy(Qt::NoFocus);
histHeader->setStyleSheet("font-size: 22px; font-weight: bold; color: white;");
histLay->addWidget(histHeader);

// Close button
m_closeHistoryButton = new AACKeyButton(tr("Close history"), m_aac, m_historyContainer);
m_closeHistoryButton->setAccessibleName(tr("Close history"));
m_closeHistoryButton->setAccessibleRole(QAccessible::Button);
m_closeHistoryButton->setDeepWell(true);
histLay->addWidget(m_closeHistoryButton);

connect(m_closeHistoryButton, &AACKeyButton::keyActivated,
        this, [this](const QString&) {
            m_historyContainer->setVisible(false);
            this->setFocus();
        });

// Actual viewer
m_historyViewer = new AACMessageHistoryViewer(m_aac->history(), m_historyContainer);
m_historyViewer->setAccessibleName(tr("Conversation history viewer"));
m_historyViewer->setAccessibleRole(QAccessible::List);
histLay->addWidget(m_historyViewer);

// Add to root layout
m_rootLayout->addWidget(m_historyContainer);

m_messageLog = new QListWidget(this);
m_messageLog->setAccessibleName(tr("Message log"));
m_messageLog->setFocusPolicy(Qt::NoFocus);
m_messageLog->setAccessibleRole(QAccessible::List);
m_messageLog->setObjectName("messageLog");
m_messageLog->setSelectionMode(QAbstractItemView::NoSelection);
m_messageLog->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
m_messageLog->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
m_messageLog->setUniformItemSizes(true);
m_messageLog->setSpacing(4);
connect(m_messageLog, &QListWidget::itemActivated,
        this, [this](QListWidgetItem*) {
            m_newMessageIndicator->clear();
        });
    m_sendToButton = new AACKeyButton(tr("Send to: Channel"), m_aac, this);
m_sendToButton->setObjectName("sendToButton");
    connect(m_sendToButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) { onSendToClicked(); });
m_sendToPanel = new QWidget(this);
m_sendToPanel->setVisible(false);
m_sendToPanel->setAccessibleName(tr("Recipient choices"));
m_sendToPanel->setFocusPolicy(Qt::NoFocus);
m_sendToPanel->setAccessibleDescription(tr("Choose where to send your message"));
m_sendToPanel->setAccessibleRole(QAccessible::Group);
auto* sendToLayout = new QVBoxLayout(m_sendToPanel);
sendToLayout->setContentsMargins(0,0,0,0);
sendToLayout->setSpacing(4);

auto* header = new QLabel(tr("Recipients"), this);
header->setAccessibleName(tr("Recipient list"));
header->setFocusPolicy(Qt::NoFocus);
sendToLayout->addWidget(header);
m_sendToChannelButton = new AACKeyButton(tr("Channel"), m_aac, this);
m_sendToChannelButton->setAccessibleName(tr("Send to channel"));
m_sendToChannelButton->setAccessibleRole(QAccessible::Button);
m_sendToChannelButton->setFocusPolicy(Qt::NoFocus)
m_sendToChannelButton->setDeepWell(true);
sendToLayout->addWidget(m_sendToChannelButton);
connect(m_sendToChannelButton, &AACKeyButton::keyActivated,
        this, [this](const QString&) {
            m_sendToChannel = true;
            m_currentUserRecipient.clear();
            updateSendToButtonLabel();
            m_sendToPanel->setVisible(false);
        });
// Persistent user buttons (initially empty)
for (const QString& u : m_usernames) {
    auto* btn = new AACKeyButton(u, m_aac, this);
    m_sendToUserButtons.append(btn);
m_sendToUserButtons->setFocusPolicy(Qt::NoFocus)
    sendToLayout->addWidget(btn);

    connect(btn, &AACKeyButton::keyActivated,
            this, [this, u](const QString&) {
                m_sendToChannel = false;
                m_currentUserRecipient = u;
                updateSendToButtonLabel();
                m_sendToPanel->setVisible(false);
            });
}
m_closeSendToPanelButton = new AACKeyButton(tr("Close panel"), m_aac, this);
m_closeSendToPanelButton->setFocusPolicy(Qt::NoFocus)
m_closeSendToPanelButton->setDeepWell(true);
sendToLayout->addWidget(m_closeSendToPanelButton);

connect(m_closeSendToPanelButton, &AACKeyButton::keyActivated,
        this, [this](const QString&) {
            m_sendToPanel->setVisible(false);
        });
    QHBoxLayout* controls = new QHBoxLayout();
    m_leaveButton = new AACKeyButton(tr("Leave"), m_aac, this);
m_leaveButton->setAccessibleName(tr("Leave channel"));
m_leaveButton->setAccessibleRole(QAccessible::Button);
m_leaveButton->setFocusPolicy(Qt::NoFocus);
m_leaveButton->setDeepWell(true);
    controls->addWidget(m_leaveButton);
    connect(m_leaveButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) { onLeaveClicked(); });

    m_volDownButton   = new AACKeyButton(tr("Quieter"), m_aac, this);
    controls->addWidget(m_volDownButton);
    connect(m_volDownButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) {
                if (!m_aac || !m_backend)
                    return;
                int v = m_aac->globalListeningVolume();
                v = qMax(0, v - 5);
                m_aac->setGlobalListeningVolume(v);
                m_backend->applyGlobalListeningVolume();
            });

    m_volUpButton     = new AACKeyButton(tr("Louder"), m_aac, this);
    controls->addWidget(m_volUpButton);
    connect(m_volUpButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) {
                if (!m_aac || !m_backend)
                    return;
                int v = m_aac->globalListeningVolume();
                v = qMin(100, v + 5);
                m_aac->setGlobalListeningVolume(v);
                m_backend->applyGlobalListeningVolume();
            });

    m_setNormalButton = new AACKeyButton(tr("Set as normal"), m_aac, this);
    controls->addWidget(m_setNormalButton);
    connect(m_setNormalButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) {
                if (!m_aac || !m_backend)
                    return;
                const int ttvol   = m_backend->currentOutputVolume();
                const int percent = invRefVolume(ttvol);
                m_aac->setGlobalListeningVolume(percent);
            });
m_historyButton = new AACKeyButton(tr("History"), m_aac, this);
m_historyButton->setAccessibleName(tr("Open conversation history"));
m_historyButton->setAccessibleRole(QAccessible::Button);
m_historyButton->setFocusPolicy(Qt::NoFocus);
m_historyButton->setDeepWell(true);
m_historyButton->setObjectName("historyButton");
controls->addWidget(m_historyButton);

connect(m_historyButton, &AACKeyButton::keyActivated,
        this, [this](const QString&) {
            if (!m_historyContainer || !m_historyViewer)
                return;

            m_historyContainer->setVisible(true);
            m_historyContainer->raise();
            m_historyViewer->setFocus();

            if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode)
                m_aac->speechEngine()->speakNotification(tr("Conversation history"));

            m_historyViewer->jumpToLastEvent();
        });
    controls->addStretch(1);

    m_rootLayout->addWidget(m_channelLabel);
    m_rootLayout->addWidget(m_eventLabel);
    m_rootLayout->addWidget(m_speakingLabel);
    m_rootLayout->addWidget(m_presenceLabel);
    m_rootLayout->addWidget(m_userList);
m_rootLayout->addSpacing(8);
    m_rootLayout->addWidget(m_transmitModeLabel);
m_rootLayout->addWidget(m_transmitStatusLabel);
m_rootLayout->addWidget(m_newMessageIndicator);
    m_rootLayout->addWidget(m_messageLog);
m_rootLayout->addSpacing(8);
    m_rootLayout->addWidget(m_sendToButton);
m_rootLayout->addSpacing(4);
m_rootLayout->addWidget(m_sendToPanel);
    m_rootLayout->addWidget(m_aacMain);
    m_rootLayout->addLayout(controls);
    m_rootLayout->addStretch(1);

    m_sendToChannel = true;
    updateSendToButtonLabel();
    updatePresenceSummary();
}
QString InChannelScreen::contextualHelp() const
{
    return tr(
        "InChannel. "
        "Press F2 to speak the current message recipient. "
        "Press Shift+F3 to speak who is here. "
        "Press F4 to speak channel status. "
        "Press Shift+F4 to speak active speakers. "
        "Press F5 to speak the last message. "
        "Press F6 to focus the message log. "
        "Press Shift+F6 to speak unread message count. "
        "Press F7 to toggle conversation history. "
        "Press Shift+F10 to speak transmit status. "
        "Press Shift+F11 to speak the channel name. "
        "Press F8 to toggle transmit. "
        "Press F9 to mute or unmute. "
        "Press F10 to silence speech. "
        "Press F11 to clear your message. "
        "Press Escape to leave the channel."
    );
}
QList<QWidget*> InChannelScreen::interactiveWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<AACKeyButton*>(m_sendToButton);
    out << const_cast<AACKeyButton*>(m_leaveButton);
    out << const_cast<AACMainScreen*>(m_aacMain);
    return out;
}

QList<QWidget*> InChannelScreen::primaryWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<AACMainScreen*>(m_aacMain);
    return out;
}

QLayout* InChannelScreen::rootLayout() const
{
    return m_rootLayout;
}

QWidget* InChannelScreen::predictiveStripContainer() const
{
    return m_aacMain->predictiveStripContainer();
}

void InChannelScreen::setChannelName(const QString& name)
{
    const QString trimmed = name.trimmed();

    if (trimmed.isEmpty()) {
        m_channelLabel->setText(tr("Channel"));
        setScreenTitle("Channel");
        emitInitialTitle();
        return;
    }

    m_channelLabel->setText(tr("Channel: %1").arg(trimmed));
    setScreenTitle(trimmed);
    emitInitialTitle();
}

void InChannelScreen::setTransmitStatus(bool enabled)
{
    m_transmitStatusLabel->setText(
        enabled ? tr("Transmit on") : tr("Not transmitting")
    );
}
void UserRowWidget::setSpeaking(bool speaking)
{
    m_micLabel->setVisible(speaking);
    setAccessibleDescription(
        speaking
        ? tr("User %1 is speaking, volume controls available").arg(username())
        : tr("User %1 is not transmitting, volume controls available").arg(username())
);

m_micLabel->setAccessibleName(
    speaking ? tr("Speaking indicator") : tr("Not transmitting indicator")
    );
}
void InChannelScreen::updateSelfVoiceState(const SelfVoiceState& state)
{
    const bool speaking = (state == SelfVoiceState::Transmitting);

    if (m_backend)
        m_userList->setUserSpeaking(m_backend->selfNickname(), speaking);

    if (state == SelfVoiceState::Transmitting)
        m_eventLabel->setText(tr("You are transmitting…"));
    else
        m_eventLabel->setText(tr("Not transmitting"));
m_eventLabel->setFocusPolicy(Qt::NoFocus);
m_eventLabel->setAccessibleName(tr("Event status"));
}
void InChannelScreen::updateOtherUserVoiceState(const OtherUserVoiceEvent& event)
{
    const bool speaking = (event.state == OtherUserVoiceState::Speaking);
    m_userList->setUserSpeaking(event.username, speaking);

    if (speaking)
        m_activeSpeakers.insert(event.userId);
    else
        m_activeSpeakers.remove(event.userId);

    if (m_activeSpeakers.isEmpty()) {
        m_speakingLabel->setText(tr("Channel quiet"));
    }
    else if (m_activeSpeakers.size() == 1) {
        m_speakingLabel->setText(tr("%1 is speaking…").arg(event.username));
    }
    else {
        m_speakingLabel->setText(
            tr("Multiple people are speaking…").arg(m_activeSpeakers.size()));
    }
}

void InChannelScreen::onUserJoined(const QString& username)
{
    // Update presence list
    if (!m_usernames.contains(username))
        m_usernames.append(username);

    updatePresenceSummary();

    // ⭐ Earcon (suppressed in fatigue mode)
    if (m_aac && m_aac->feedbackEngine() && !m_aac->modes().fatigueMode) {
        m_aac->feedbackEngine()->playUserJoin();
    }

    // ⭐ Optional: speech announcement
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        m_aac->speechEngine()->speakNotification(
            tr("%1 joined the channel").arg(username)
        );
    }
}

void InChannelScreen::onUserLeft(const QString& username)
{
    // Update presence list
    m_usernames.removeAll(username);
    updatePresenceSummary();

    // ⭐ Earcon (suppressed in fatigue mode)
    if (m_aac && m_aac->feedbackEngine() && !m_aac->modes().fatigueMode) {
        m_aac->feedbackEngine()->playUserLeave();
    }

    // ⭐ Optional: speech announcement
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        m_aac->speechEngine()->speakNotification(
            tr("%1 left the channel").arg(username)
        );
    }
}
void InChannelScreen::setEventMessage(const QString& msg)
{
    m_eventLabel->setText(msg);
}

void InChannelScreen::setUsers(const QList<QString>& usernames)
{
    m_usernames = usernames;

    QList<int> ids;
    for (const QString& name : usernames)
        ids.append(m_backend->userIdForName(name));

    if (m_userList)
        m_userList->setUsers(usernames, ids, m_aac, m_backend);

    updatePresenceSummary();
}

void InChannelScreen::setTransmitModeLabel(BackendAdapter::AACTransmitMode mode)
{
    switch (mode) {
    case BackendAdapter::AACTransmitMode::TapToToggle:
        m_transmitModeLabel->setText(tr("Transmit mode: Toggle transmit"));
        break;
    case BackendAdapter::AACTransmitMode::Continuous:
        m_transmitModeLabel->setText(tr("Transmit mode: Continuous"));
        break;
    case BackendAdapter::AACTransmitMode::VoiceActivation:
        m_transmitModeLabel->setText(tr("Transmit mode: Voice activation"));
        break;
    case BackendAdapter::AACTransmitMode::AutoSilence:
        m_transmitModeLabel->setText(tr("Transmit mode: Auto‑silence"));
        break;
    }
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode)
        m_aac->speechEngine()->speakNotification(m_transmitModeLabel->text());
}

void InChannelScreen::onAACMessageReceived(const AACMessage& msg)
{
    // Earcon
    if (m_aac && m_aac->feedbackEngine() && !m_aac->modes().fatigueMode) {
        if (msg.isPrivate)
            m_aac->feedbackEngine()->playPrivateMessage();
        else
            m_aac->feedbackEngine()->playIncomingMessage();
    }

    // Speech announcement
    if (m_aac && m_aac->speechEngine() && !m_aac->modes().fatigueMode) {
        if (msg.isPrivate)
            m_aac->speechEngine()->speakNotification(
                tr("Private message from %1").arg(msg.fromUsername)
            );
        else
            m_aac->speechEngine()->speakNotification(
                tr("Message from %1").arg(msg.fromUsername)
            );
    }

    // Visual indicator
    if (msg.isPrivate)
        m_newMessageIndicator->setText(
            tr("🔒 Private message from %1").arg(msg.fromUsername)
        );
    else
        m_newMessageIndicator->setText(
            tr("📨 Message from %1").arg(msg.fromUsername)
        );

    // Build message line
    QString prefix;
    if (msg.isPrivate) {
        if (msg.fromUserId == m_backend->selfUserId())
            prefix = tr("You → %1 (private): ").arg(m_currentUserRecipient);
        else
            prefix = tr("(Private) %1 → You: ").arg(msg.fromUsername);
    } else {
        if (msg.fromUserId == m_backend->selfUserId())
            prefix = tr("You → Channel: ");
        else
            prefix = tr("%1: ").arg(msg.fromUsername);
    }

QString timestamp = QDateTime::currentDateTime().toString("HH:mm");
QString line = QString("[%1] %2").arg(timestamp, prefix + msg.text);

auto* item = new QListWidgetItem(line);
item->setData(Qt::AccessibleDescriptionRole,
              tr("%1 says at %2: %3")
                  .arg(msg.fromUsername)
                  .arg(timestamp)
                  .arg(msg.text));
m_messageLog->addItem(item);
++m_newMessageCount;
while (m_messageLog->count() > 20)
    delete m_messageLog->takeItem(0);

m_messageLog->scrollToBottom();

// Append to AACMessageHistory (viewer updates automatically)
if (m_aac && m_aac->history()) {
    AACMessageHistoryEvent ev;
    ev.username  = msg.fromUsername;
    ev.text      = msg.text;
    ev.isPrivate = msg.isPrivate;
    ev.timestamp = QDateTime::currentDateTime();

    m_aac->history()->append(ev);
}
}
void InChannelScreen::updateMessageLog()
{
    // No-op: QListWidget updates incrementally
}

void InChannelScreen::onLeaveClicked()
{
    emit leaveChannelRequested();
}

void InChannelScreen::onSendToClicked()
{
    // Toggle visibility
    bool show = !m_sendToPanel->isVisible();
    m_sendToPanel->setVisible(show);
if (show && m_aac && m_aac->speechEngine()) {
    m_aac->speechEngine()->speakNotification(tr("Choose message recipient"));
}
    if (!show)
        return;
}

void InChannelScreen::updatePresenceSummary()
{
    if (m_usernames.isEmpty()) {
        m_presenceLabel->setText(tr("People here: (none)"));
        return;
    }

    m_presenceLabel->setText(
        tr("People here: %1").arg(m_usernames.join(", ")));
}

void InChannelScreen::updateSendToButtonLabel()
{
    if (m_sendToChannel || m_currentUserRecipient.isEmpty()) {
        m_sendToButton->setText(tr("Send to: Channel"));
    } else {
        m_sendToButton->setText(tr("Send to: %1").arg(m_currentUserRecipient));
    }
}