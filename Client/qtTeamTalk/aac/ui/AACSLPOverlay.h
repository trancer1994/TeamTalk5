#pragma once

#include <QWidget>
#include <QPointer>

class AACInputController;

class AACSLPOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit AACSLPOverlay(AACInputController* controller,
                           QWidget* parent = nullptr);

    void setOverlayVisible(bool visible);

signals:
    void analyticsUpdated();
    void usageUpdated();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPointer<AACInputController> m_controller;

    struct OverlayState {
        int totalActivations = 0;
        int totalIntents     = 0;
        int totalErrors      = 0;
        int totalCorrections = 0;

        QString currentProfile;
        int dwellTimeMs      = 0;
        int scanStepMs       = 0;
    } m_state;

    void hookIntoCore();
    void refreshFromCore();

    void drawUsagePanel(QPainter& p, const QRect& r);
    void drawProfilePanel(QPainter& p, const QRect& r);
    void drawScanningPanel(QPainter& p, const QRect& r);
    void drawRemotePanel(QPainter& p, const QRect& r);

    // Remote scaffolding (temporary guidance)
    void remoteJumpToGroup(const QString& groupName);
    void remoteSetHighlight(int row, int col);
    void remoteActivateCurrent();
};
