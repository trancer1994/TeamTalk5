#include "AACSLPOverlay.h"
#include "AACInputController.h"

#include <QPainter>
#include <QPaintEvent>

AACSLPOverlay::AACSLPOverlay(AACInputController* controller,
                             QWidget* parent)
    : QWidget(parent)
    , m_controller(controller)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setOverlayVisible(false);
    hookIntoCore();
}

void AACSLPOverlay::setOverlayVisible(bool visible)
{
    setVisible(visible);
    if (visible)
        refreshFromCore();
}

void AACSLPOverlay::hookIntoCore()
{
    if (!m_controller)
        return;

    auto& core = m_controller->core();

    core.onUsageUpdated = [this](const AACInputController::Core::UsageStats& usage) {
        m_state.totalActivations = usage.totalActivations;
        emit usageUpdated();
        update();
    };

    core.onAnalyticsUpdated = [this](const AACInputController::Core::SessionAnalytics& a) {
        m_state.totalIntents     = a.totalIntents;
        m_state.totalErrors      = a.totalErrors;
        m_state.totalCorrections = a.totalCorrections;
        emit analyticsUpdated();
        update();
    };

    core.onLayoutChanged = [this](const QVector<QVector<AACNode*>>&) {
        refreshFromCore();
        update();
    };

    core.setSLPRemoteActive(true);
    refreshFromCore();
}

void AACSLPOverlay::refreshFromCore()
{
    if (!m_controller)
        return;

    auto& core = m_controller->core();

    m_state.dwellTimeMs = core.dwellTimeMs;
    m_state.scanStepMs  = core.scanProfile.scanStepMs;

    using Profile = AACInputController::Core::Profile;
    switch (core.profile) {
    case Profile::Default:           m_state.currentProfile = "Default"; break;
    case Profile::Blind:             m_state.currentProfile = "Blind"; break;
    case Profile::LowVision:         m_state.currentProfile = "LowVision"; break;
    case Profile::MotorImpaired:     m_state.currentProfile = "MotorImpaired"; break;
    case Profile::CognitiveImpaired: m_state.currentProfile = "CognitiveImpaired"; break;
    case Profile::SwitchUser:        m_state.currentProfile = "SwitchUser"; break;
    case Profile::EyeTrackingUser:   m_state.currentProfile = "EyeTrackingUser"; break;
    }
}

void AACSLPOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if (!isVisible())
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRect full = rect();
    int h = full.height() / 4;

    QRect usageRect    = QRect(full.left(), full.top(), full.width(), h);
    QRect profileRect  = QRect(full.left(), full.top() + h, full.width(), h);
    QRect scanRect     = QRect(full.left(), full.top() + 2*h, full.width(), h);
    QRect remoteRect   = QRect(full.left(), full.top() + 3*h, full.width(), h);

    drawUsagePanel(p, usageRect);
    drawProfilePanel(p, profileRect);
    drawScanningPanel(p, scanRect);
    drawRemotePanel(p, remoteRect);
}

void AACSLPOverlay::drawUsagePanel(QPainter& p, const QRect& r)
{
    p.save();
    p.setPen(Qt::white);
    p.setBrush(QColor(0, 0, 0, 160));
    p.drawRect(r);

    p.drawText(r.adjusted(8, 8, -8, -8),
               Qt::AlignLeft | Qt::AlignTop,
               QStringLiteral("Usage\nTotal activations: %1\nTotal intents: %2")
               .arg(m_state.totalActivations)
               .arg(m_state.totalIntents));
    p.restore();
}

void AACSLPOverlay::drawProfilePanel(QPainter& p, const QRect& r)
{
    p.save();
    p.setPen(Qt::white);
    p.setBrush(QColor(0, 0, 40, 160));
    p.drawRect(r);

    p.drawText(r.adjusted(8, 8, -8, -8),
               Qt::AlignLeft | Qt::AlignTop,
               QStringLiteral("Profile\nCurrent: %1\nDwell: %2 ms")
               .arg(m_state.currentProfile)
               .arg(m_state.dwellTimeMs));
    p.restore();
}

void AACSLPOverlay::drawScanningPanel(QPainter& p, const QRect& r)
{
    p.save();
    p.setPen(Qt::white);
    p.setBrush(QColor(0, 40, 0, 160));
    p.drawRect(r);

    p.drawText(r.adjusted(8, 8, -8, -8),
               Qt::AlignLeft | Qt::AlignTop,
               QStringLiteral("Scanning\nStep: %1 ms")
               .arg(m_state.scanStepMs));
    p.restore();
}

void AACSLPOverlay::drawRemotePanel(QPainter& p, const QRect& r)
{
    p.save();
    p.setPen(Qt::white);
    p.setBrush(QColor(40, 0, 0, 160));
    p.drawRect(r);

    p.drawText(r.adjusted(8, 8, -8, -8),
               Qt::AlignLeft | Qt::AlignTop,
               QStringLiteral("Remote scaffolding\n"
                              "SLP can jump to groups or set highlight\n"
                              "(wire to UI controls, not here)"));
    p.restore();
}

void AACSLPOverlay::remoteJumpToGroup(const QString& groupName)
{
    if (!m_controller)
        return;

    auto& core = m_controller->core();
    using SG = AACInputController::Core::SemanticGroup;

    SG group = SG::Emotion;
    if (groupName == "Need")       group = SG::Need;
    else if (groupName == "Social")   group = SG::Social;
    else if (groupName == "Question") group = SG::Question;
    else if (groupName == "Control")  group = SG::Control;

    core.remoteJumpToGroup(group);
}

void AACSLPOverlay::remoteSetHighlight(int row, int col)
{
    if (!m_controller)
        return;

    m_controller->core().remoteSetHighlight(row, col);
}

void AACSLPOverlay::remoteActivateCurrent()
{
    if (!m_controller)
        return;

    m_controller->core().remoteActivateCurrent();
}
