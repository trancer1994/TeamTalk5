#include "InChannelScreen.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>

#include "aac/AACMainScreen.h"

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
setFocusPolicy(Qt::StrongFocus);
setAccessibleName(username);

        QHBoxLayout* lay = new QHBoxLayout(this);
        lay->setContentsMargins(4, 2, 4, 2);
        lay->setSpacing(8);

        m_nameLabel = new QLabel(username, this);
        m_micLabel  = new QLabel(this);
        m_micLabel->setPixmap(QPixmap(":/icons/mic.png").scaled(24, 24));
        m_micLabel->setVisible(false);

        // AAC‑native volume buttons
        m_quieterBtn = new AACKeyButton(tr("−"), m_aac, this);
        m_louderBtn  = new AACKeyButton(tr("+"), m_aac, this);
        m_resetBtn   = new AACKeyButton(tr("Normal"), m_aac, this);

        m_quieterBtn->setMinimumWidth(60);
        m_louderBtn->setMinimumWidth(60);
        m_resetBtn->setMinimumWidth(90);

        connect(m_quieterBtn, &AACKeyButton::keyActivated,
                this, [this](const QString&) { adjustVolume(-5); });

        connect(m_louderBtn, &AACKeyButton::keyActivated,
                this, [this](const QString&) { adjustVolume(+5); });

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
        m_layout->setSpacing(4);
    }

void setUsers(const QList<QString>& usernames,
              const QList<int>& userIds,
              AACAccessibilityManager* aac,
              BackendAdapter* backend)
{
    // Clear layout and rows
    while (QLayoutItem* item = m_layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    m_rows.clear();

    const int count = qMin(usernames.size(), userIds.size());
    for (int i = 0; i < count; ++i) {
        auto* row = new UserRowWidget(usernames[i],
                                      userIds[i],
                                      aac,
                                      backend,
                                      this);
        m_rows.append(row);
        m_layout->addWidget(row);
    }

    m_layout->addStretch(1);
}

    void setUserSpeaking(const QString& username, bool speaking)
    {
        for (auto* row : m_rows) {
            if (row->username() == username) {
                row->setSpeaking(speaking);
                return;
            }
        }
    }
void InChannelScreen::keyPressEvent(QKeyEvent* e)
{
    // ⭐ F6: Jump to message log (blind‑friendly, AAC‑safe)
    if (e->key() == Qt::Key_F6) {
        if (m_messageLog) {
            m_messageLog->setFocus();
        }
        return;
    }

    QWidget::keyPressEvent(e);
}
    void keyPressEvent(QKeyEvent* e) override
    {
        if (m_rows.isEmpty()) {
            QWidget::keyPressEvent(e);
            return;
        }

        int idx = m_rows.indexOf(qobject_cast<UserRowWidget*>(focusWidget()));
        if (idx < 0)
            idx = 0;

        switch (e->key()) {
        case Qt::Key_Up:
            idx = qMax(0, idx - 1);
            m_rows[idx]->setFocus();
            return;

        case Qt::Key_Down:
            idx = qMin(m_rows.size() - 1, idx + 1);
            m_rows[idx]->setFocus();
            return;

        case Qt::Key_Home:
            m_rows.first()->setFocus();
            return;

        case Qt::Key_End:
            m_rows.last()->setFocus();
            return;
        }

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
    m_rootLayout->setSpacing(8);

    m_channelLabel   = new QLabel(tr("Channel"), this);
    m_eventLabel     = new QLabel(this);
    m_speakingLabel  = new QLabel(tr("No one is speaking"), this);

m_speakingLabel->setFocusPolicy(Qt::StrongFocus);
m_speakingLabel->setAccessibleName(tr("Speaking status"));

    m_presenceLabel  = new QLabel(tr("People here: (none)"), this);
    m_transmitModeLabel = new QLabel(tr("Transmit mode: Tap to toggle"), this);
m_transmitStatusLabel = new QLabel(tr("Transmit off"), this);
m_transmitStatusLabel->setAccessibleName(tr("Transmit status"));
m_transmitStatusLabel->setFocusPolicy(Qt::StrongFocus);
m_transmitStatusLabel->setVisible(false);

m_newMessageIndicator = new QLabel(this);
m_newMessageIndicator->setText(QString());
m_newMessageIndicator->setAccessibleName(tr("New message indicator"));
m_newMessageIndicator->setFocusPolicy(Qt::NoFocus);

    m_presenceLabel->setFocusPolicy(Qt::StrongFocus);
    m_presenceLabel->setAccessibleName(tr("People here"));

    m_userList = new UserListWidget(this);

// NEW: instantiate AACMainScreen
m_aacMain = new AACMainScreen(m_aac, this);

    // NEW: message log
    m_messageLog = new QLabel(this);
    m_messageLog->setWordWrap(true);
    m_messageLog->setText(tr("No messages yet"));
    m_messageLog->setAccessibleName(tr("Message log"));
    m_messageLog->setFocusPolicy(Qt::NoFocus);

    m_sendToButton = new AACKeyButton(tr("Send to: Channel"), m_aac, this);
    connect(m_sendToButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) { onSendToClicked(); });
m_sendToPanel = new QWidget(this);
m_sendToPanel->setVisible(false);
m_sendToPanel->setAccessibleName(tr("Recipient choices"));

auto* sendToLayout = new QVBoxLayout(m_sendToPanel);
sendToLayout->setContentsMargins(0,0,0,0);
sendToLayout->setSpacing(4);

    QHBoxLayout* controls = new QHBoxLayout();
    m_leaveButton = new AACKeyButton(tr("Leave"), m_aac, this);
m_leaveButton->setDeepWell(true);
    m_volDownButton   = new AACKeyButton(tr("Quieter"), m_aac, this);
    m_volUpButton     = new AACKeyButton(tr("Louder"), m_aac, this);
    m_setNormalButton = new AACKeyButton(tr("Set as normal"), m_aac, this);

    connect(m_leaveButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) { onLeaveClicked(); });

    connect(m_volDownButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) {
                if (!m_aac || !m_backend)
                    return;
                int v = m_aac->globalListeningVolume();
                v = qMax(0, v - 5);
                m_aac->setGlobalListeningVolume(v);
                m_backend->applyGlobalListeningVolume();
            });

    connect(m_volUpButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) {
                if (!m_aac || !m_backend)
                    return;
                int v = m_aac->globalListeningVolume();
                v = qMin(100, v + 5);
                m_aac->setGlobalListeningVolume(v);
                m_backend->applyGlobalListeningVolume();
            });

    connect(m_setNormalButton, &AACKeyButton::keyActivated,
            this, [this](const QString&) {
                if (!m_aac || !m_backend)
                    return;
                const int ttvol   = m_backend->currentOutputVolume();
                const int percent = invRefVolume(ttvol);
                m_aac->setGlobalListeningVolume(percent);
            });

    controls->addWidget(m_leaveButton);
    controls->addWidget(m_volDownButton);
    controls->addWidget(m_volUpButton);
    controls->addWidget(m_setNormalButton);
    controls->addStretch(1);

    m_rootLayout->addWidget(m_channelLabel);
    m_rootLayout->addWidget(m_eventLabel);
    m_rootLayout->addWidget(m_speakingLabel);
    m_rootLayout->addWidget(m_presenceLabel);
    m_rootLayout->addWidget(m_userList);
    m_rootLayout->addWidget(m_transmitModeLabel);
m_rootLayout->addWidget(m_transmitStatusLabel);
m_rootLayout->addWidget(m_newMessageIndicator);
    m_rootLayout->addWidget(m_messageLog);
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
    return tr("InChannel. "
               "Press F8 to toggle transmit. "
               "Press F9 to mute or unmute. "
               "Press F10 to silence speech. "
               "Press F11 to clear your message. "
               "Press Escape to leave the channel.");
}
QList<QWidget*> InChannelScreen::interactiveWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<AACMainScreen*>(m_aacMain);
    out << const_cast<AACKeyButton*>(m_sendToButton);
    out << const_cast<AACKeyButton*>(m_leaveButton);
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
        enabled ? tr("Transmit on") : tr("Transmit off")
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
        m_eventLabel->setText(tr("Silent"));
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
        m_speakingLabel->setText(tr("No one is speaking"));
    }
    else if (m_activeSpeakers.size() == 1) {
        m_speakingLabel->setText(tr("%1 is speaking…").arg(event.username));
    }
    else {
        m_speakingLabel->setText(
            tr("%1 people are speaking…").arg(m_activeSpeakers.size()));
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
        m_transmitModeLabel->setText(tr("Transmit mode: Tap to toggle"));
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

    QString line = prefix + msg.text;

    // Append to buffer
    m_messageBuffer.append(line);
    while (m_messageBuffer.size() > 5)
        m_messageBuffer.removeFirst();

    updateMessageLog();
}
void InChannelScreen::updateMessageLog()
{
    if (m_messageBuffer.isEmpty()) {
        m_messageLog->setText(tr("No messages yet"));
        return;
    }

    m_messageLog->setText(m_messageBuffer.join("\n"));
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

    if (auto* lay = qobject_cast<QVBoxLayout*>(m_sendToPanel->layout())) {
        if (auto* item = lay->itemAt(0)) {
            if (auto* btn = qobject_cast<AACKeyButton*>(item->widget())) {
                btn->setFocus();   // ⭐ This is the important line
            }
        }
    }

    // Rebuild panel
    QLayoutItem* child;
    while ((child = m_sendToPanel->layout()->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    auto* lay = static_cast<QVBoxLayout*>(m_sendToPanel->layout());

    // Channel button
    {
        auto* btn = new AACKeyButton(tr("Channel"), m_aac, this);
btn->setDeepWell(true);
        lay->addWidget(btn);
        connect(btn, &AACKeyButton::keyActivated,
                this, [this](const QString&) {
                    m_sendToChannel = true;
                    m_currentUserRecipient.clear();
                    updateSendToButtonLabel();
                    m_sendToPanel->setVisible(false);
                });
    }

    // User buttons
    for (const QString& u : m_usernames) {
        auto* btn = new AACKeyButton(u, m_aac, this);
        lay->addWidget(btn);
        connect(btn, &AACKeyButton::keyActivated,
                this, [this, u](const QString&) {
                    m_sendToChannel = false;
                    m_currentUserRecipient = u;
                    updateSendToButtonLabel();
                    m_sendToPanel->setVisible(false);
                });
    }
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