#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QElapsedTimer>

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
    void onHighlightChanged();
    void onItemActivated(QListWidgetItem* item);

private:
    void setupUi();
    void ensureHighlightVisible();
    bool acceptInput();

private:
    QListWidget* m_list = nullptr;
    QPushButton* m_joinButton = nullptr;
    QPushButton* m_backButton = nullptr;
    QLabel* m_statusLabel = nullptr;

    bool m_transitioning = false;
    QElapsedTimer m_debounce;
};
