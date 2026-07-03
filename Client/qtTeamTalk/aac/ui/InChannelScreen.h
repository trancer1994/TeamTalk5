#pragma once

#include "AACScreenBase.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSet>
#include <QList>

#include "aac/AACFramework.h"      // AACScreenAdapter, AACAccessibilityManager
#include "aac/AACKeyButton.h"
#include "backend/BackendAdapter.h"

class AACMainScreen;
class UserListWidget;

struct SelfVoiceState;
struct OtherUserVoiceEvent;

class InChannelScreen : public AACScreenBase, public AACScreenAdapter
{
    Q_OBJECT
public:
    explicit InChannelScreen(AACAccessibilityManager* aac,
                             BackendAdapter* backend,
                             QWidget* parent = nullptr);

    // AACScreenAdapter overrides
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override;

    void setChannelName(const QString& name);

signals:
    void leaveChannelRequested();

public slots:
    void updateSelfVoiceState(const SelfVoiceState& state);
    void updateOtherUserVoiceState(const OtherUserVoiceEvent& event);
    void setEventMessage(const QString& msg);
    void setUsers(const QList<QString>& usernames);
    void setTransmitModeLabel(BackendAdapter::AACTransmitMode mode);

private slots:
    void onLeaveClicked();
    void onSendToClicked();
    void onAACMessageReceived(const AACMessage& msg);

private:
    void updatePresenceSummary();
    void updateSendToButtonLabel();
    void updateMessageLog();   // NEW

    AACAccessibilityManager* m_aac = nullptr;
    BackendAdapter* m_backend = nullptr;

    QVBoxLayout* m_rootLayout = nullptr;
    AACMainScreen* m_aacMain = nullptr;

    QLabel* m_channelLabel = nullptr;
    QLabel* m_eventLabel = nullptr;
    QLabel* m_speakingLabel = nullptr;
    QLabel* m_presenceLabel = nullptr;
    QLabel* m_transmitModeLabel = nullptr;

    UserListWidget* m_userList = nullptr;

QLabel* m_newMessageIndicator = nullptr;
    QLabel* m_messageLog = nullptr;        // NEW
    QStringList m_messageBuffer;           // NEW
QWidget* m_sendToPanel = nullptr;

    AACKeyButton* m_sendToButton = nullptr;
    AACKeyButton* m_leaveButton = nullptr;
    AACKeyButton* m_volDownButton = nullptr;
    AACKeyButton* m_volUpButton = nullptr;
    AACKeyButton* m_setNormalButton = nullptr;

    QSet<int> m_activeSpeakers;
    QList<QString> m_usernames;

    bool m_sendToChannel = true;
    QString m_currentUserRecipient;
};
