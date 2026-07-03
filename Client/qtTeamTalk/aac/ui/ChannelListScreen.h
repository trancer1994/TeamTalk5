#pragma once
#include "AACScreenBase.h"

struct ChannelInfo {
    QString id;
    QString name;
};

class AACKeyButton;

class ChannelListScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit ChannelListScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setChannels(const QList<ChannelInfo>& channels);

signals:
    void channelChosen(const QString& id);
    void backRequested();
    void refreshRequested();

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    QList<ChannelInfo> m_channels;
    QVector<AACKeyButton*> m_channelButtons;

    void announceChannelCount(int count);
};
