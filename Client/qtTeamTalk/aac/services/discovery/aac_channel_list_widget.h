#pragma once

#include <QWidget>

class QListWidget;
class QPushButton;
class QLabel;

class AACChannelListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AACChannelListWidget(QWidget* parent = nullptr);

    void setChannels(const QStringList& channels);
    QString selectedChannel() const;

signals:
    void joinRequested(const QString& channelName);
    void backRequested();

private slots:
    void onJoinClicked();

private:
    void setupUi();

private:
    QListWidget* m_list = nullptr;
    QPushButton* m_joinButton = nullptr;
    QPushButton* m_backButton = nullptr;
    QLabel* m_statusLabel = nullptr;
};
