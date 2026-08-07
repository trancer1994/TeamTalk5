#include "AACInputController.h"
#include "AACAccessibilityManager.h"
#include "AACKeyButton.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QDebug>

// Simple map between AACNode and AACKeyButton (Qt side)
static QHash<AACNode*, AACKeyButton*> g_nodeToButton;

AACInputController::AACInputController(AACAccessibilityManager* accessibility,
                                       QObject* parent)
    : QObject(parent)
    , m_accessibility(accessibility)
{
    connect(&m_dwellTimer, &QTimer::timeout,
            this, &AACInputController::onDwellTimeout);

    connect(&m_scanStepTimer, &QTimer::timeout,
            this, &AACInputController::onScanStepTimeout);

    //
    // Wire core callbacks to Qt signals
    //
    m_core.onHighlightChanged = [this](AACNode* node) {
        QWidget* w = nodeToWidget(node);

    if (w && m_accessibility && m_accessibility->modes().scanning) {
        if (m_accessibility->registry()) {
            AACElementMetadata md = m_accessibility->registry()->metadata(w);

            // Fatigue suppression
            if (!(m_accessibility->modes().fatigueMode && md.suppressInFatigue)) {
                if (!md.helpText.isEmpty() && m_accessibility->speechEngine()) {
                    m_accessibility->speechEngine()->speakScanningItem(md.helpText);
                }
            }
        }
    }

        emit highlightChanged(w);

        if (auto* btn = qobject_cast<AACKeyButton*>(w)) {
            emit itemEntered(btn);

            if (m_core.semantics.contains(node)) {
                const auto& d = m_core.semantics[node];
                emit groupEntered(d.group == Core::SemanticGroup::Emotion   ? "Emotion"  :
                                  d.group == Core::SemanticGroup::Need      ? "Need"     :
                                  d.group == Core::SemanticGroup::Social    ? "Social"   :
                                  d.group == Core::SemanticGroup::Question  ? "Question" :
                                  d.group == Core::SemanticGroup::Control   ? "Control"  :
                                  d.group == Core::SemanticGroup::Letter    ? "Letter"   :
                                  d.group == Core::SemanticGroup::Number    ? "Number"   :
                                  d.group == Core::SemanticGroup::Symbol    ? "Symbol"   :
                                  "Unknown");
            }

            emit rowEntered(m_core.row);
        }

        // Dwell auto‑start
        if (m_core.dwellEnabled &&
            m_core.state == Core::ControllerState::Navigation) {
            m_dwellTimer.start(m_core.dwellTimeMs);
        }
    };

    m_core.onActivationRequested = [this](AACNode* node) {
        if (auto* btn = qobject_cast<AACKeyButton*>(nodeToWidget(node)))
            emit activationRequested(btn);
    };

    m_core.onSemanticHighlightChanged = [this](const QString& tag) {
        emit semanticHighlightChanged(tag);
    };

    m_core.onRowEntered = [this](int r) {
        emit rowEntered(r);
    };

    m_core.onGroupEntered = [this](Core::SemanticGroup g) {
        QString name;
        switch (g) {
        case Core::SemanticGroup::Emotion:  name = "Emotion";  break;
        case Core::SemanticGroup::Need:     name = "Need";     break;
        case Core::SemanticGroup::Social:   name = "Social";   break;
        case Core::SemanticGroup::Question: name = "Question"; break;
        case Core::SemanticGroup::Control:  name = "Control";  break;
        case Core::SemanticGroup::Letter:   name = "Letter";   break;
        case Core::SemanticGroup::Number:   name = "Number";   break;
        case Core::SemanticGroup::Symbol:   name = "Symbol";   break;
        }
        emit groupEntered(name);
    };

    m_core.onItemEntered = [this](AACNode* node) {
        if (auto* btn = qobject_cast<AACKeyButton*>(nodeToWidget(node)))
            emit itemEntered(btn);
    };

    m_core.onItemActivated = [this](AACNode* node) {
        if (auto* btn = qobject_cast<AACKeyButton*>(nodeToWidget(node)))
            emit itemActivated(btn);
    };
}

//
// Qt ↔ AAC mapping
//
AACNode* AACInputController::widgetToNode(QWidget* w)
{
    if (!w)
        return nullptr;

    auto* btn = qobject_cast<AACKeyButton*>(w);
    if (!btn)
        return nullptr;

    auto* node = new AACNode;
    node->id    = btn->objectName();
    node->label = btn->text();
    g_nodeToButton.insert(node, btn);
    return node;
}

QWidget* AACInputController::nodeToWidget(AACNode* n)
{
    return g_nodeToButton.value(n, nullptr);
}

void AACInputController::rebuildCoreLayout(QGridLayout* grid,
                                           QHBoxLayout* curatedStrip,
                                           QHBoxLayout* controlRow)
{
    m_core.layout.clear();
    m_core.curatedStrip.clear();
    m_core.controlRow.clear();
    m_core.semantics.clear();

    int currentRowIndex = 0;

    // Curated strip row
    if (curatedStrip) {
        QVector<AACNode*> rowNodes;
        int count = curatedStrip->count();
        for (int i = 0; i < count; ++i) {
            if (auto* item = curatedStrip->itemAt(i)) {
                if (auto* w = item->widget()) {
                    if (auto* btn = qobject_cast<AACKeyButton*>(w)) {
                        AACNode* node = widgetToNode(btn);
                        m_core.curatedStrip.append(node);
                        rowNodes.append(node);

                        Core::SemanticDescriptor d;
                        const QString t = btn->text();
                        if (t == "🙂") {
                            d.group  = Core::SemanticGroup::Emotion;
                            d.tag    = "emotion_happy";
                            d.phrase = "I feel happy";
                            d.intensity = 1;
                        } else if (t == "😢") {
                            d.group  = Core::SemanticGroup::Emotion;
                            d.tag    = "emotion_sad";
                            d.phrase = "I feel sad";
                            d.intensity = 1;
                        } else if (t == "😡") {
                            d.group  = Core::SemanticGroup::Emotion;
                            d.tag    = "emotion_angry";
                            d.phrase = "I feel angry";
                            d.intensity = 2;
                        } else if (t == "😱") {
                            d.group  = Core::SemanticGroup::Emotion;
                            d.tag    = "emotion_scared";
                            d.phrase = "I feel scared";
                            d.intensity = 2;
                        } else if (t == "👍") {
                            d.group  = Core::SemanticGroup::Social;
                            d.tag    = "yes";
                            d.phrase = "Yes";
                            d.politeness = 1;
                        } else if (t == "👎") {
                            d.group  = Core::SemanticGroup::Social;
                            d.tag    = "no";
                            d.phrase = "No";
                        } else if (t == "❤️") {
                            d.group  = Core::SemanticGroup::Emotion;
                            d.tag    = "love";
                            d.phrase = "I love this";
                            d.intensity = 2;
                        } else if (t == "❓") {
                            d.group  = Core::SemanticGroup::Question;
                            d.tag    = "question";
                            d.phrase = "I have a question";
                        } else if (t == "💧") {
                            d.group  = Core::SemanticGroup::Need;
                            d.tag    = "need_water";
                            d.phrase = "I need a drink";
                            d.urgency = 1;
                        } else if (t == "🍽️") {
                            d.group  = Core::SemanticGroup::Need;
                            d.tag    = "need_food";
                            d.phrase = "I need food";
                            d.urgency = 1;
                        } else if (t == "🛏️") {
                            d.group  = Core::SemanticGroup::Need;
                            d.tag    = "need_rest";
                            d.phrase = "I need rest";
                        } else if (t == "🆘") {
                            d.group  = Core::SemanticGroup::Need;
                            d.tag    = "need_help";
                            d.phrase = "I need help";
                            d.urgency = 3;
                        } else if (t == "👋") {
                            d.group  = Core::SemanticGroup::Social;
                            d.tag    = "hello";
                            d.phrase = "Hello";
                            d.politeness = 1;
                        } else if (t == "🙏") {
                            d.group  = Core::SemanticGroup::Social;
                            d.tag    = "please";
                            d.phrase = "Please";
                            d.politeness = 2;
                        } else if (t == "🙇") {
                            d.group  = Core::SemanticGroup::Social;
                            d.tag    = "sorry";
                            d.phrase = "I'm sorry";
                            d.politeness = 2;
                        } else if (t == "🤝") {
                            d.group  = Core::SemanticGroup::Social;
                            d.tag    = "thank_you";
                            d.phrase = "Thank you";
                            d.politeness = 2;
                        }

                        if (!d.tag.isEmpty())
                            m_core.semantics.insert(node, d);
                    }
                }
            }
        }
        if (!rowNodes.isEmpty()) {
            m_core.layout.append(rowNodes);
            ++currentRowIndex;
        }
    }

    // Keyboard grid rows
    if (grid) {
        int rows = grid->rowCount();
        int cols = grid->columnCount();
        for (int r = 0; r < rows; ++r) {
            QVector<AACNode*> rowNodes;
            for (int c = 0; c < cols; ++c) {
                if (auto* item = grid->itemAtPosition(r, c)) {
                    if (auto* w = item->widget()) {
                        if (auto* btn = qobject_cast<AACKeyButton*>(w)) {
                            AACNode* node = widgetToNode(btn);
                            rowNodes.append(node);
                        }
                    }
                }
            }
            if (!rowNodes.isEmpty()) {
                m_core.layout.append(rowNodes);
                ++currentRowIndex;
            }
        }
    }

    // Control row
    if (controlRow) {
        QVector<AACNode*> rowNodes;
        int count = controlRow->count();
        for (int i = 0; i < count; ++i) {
            if (auto* item = controlRow->itemAt(i)) {
                if (auto* w = item->widget()) {
                    if (auto* btn = qobject_cast<AACKeyButton*>(w)) {
                        AACNode* node = widgetToNode(btn);
                        rowNodes.append(node);

                        Core::SemanticDescriptor d;
                        d.group = Core::SemanticGroup::Control;
                        const QString t = btn->text();
                        if (t.compare("Space", Qt::CaseInsensitive) == 0)
                            d.tag = "control_space";
                        else if (t.compare("Backspace", Qt::CaseInsensitive) == 0)
                            d.tag = "control_backspace";
                        else if (t.compare("Enter", Qt::CaseInsensitive) == 0)
                            d.tag = "control_enter";
                        else if (t.compare("Done", Qt::CaseInsensitive) == 0)
                            d.tag = "control_done";

                        if (!d.tag.isEmpty())
                            m_core.semantics.insert(node, d);
                    }
                }
            }
        }
        if (!rowNodes.isEmpty()) {
            m_core.controlRow = rowNodes;
            m_core.layout.append(rowNodes);
            ++currentRowIndex;
        }
    }

// --- Metadata-aware predictive strip scanning (word predictions) ---
if (auto* ps = screen->findChild<PredictiveStrip*>("predictiveStrip")) {
    QVector<AACNode*> rowNodes;

    for (QWidget* w : ps->interactiveWidgets()) {
        if (auto* btn = qobject_cast<AACKeyButton*>(w)) {
            AACNode* node = widgetToNode(btn);
            rowNodes.append(node);

            Core::SemanticDescriptor d;
            d.group  = Core::SemanticGroup::Symbol;   // or Prediction group
            d.tag    = "prediction_word";
            d.phrase = btn->text();                  // speak the word itself

            m_core.semantics.insert(node, d);
        }
    }

    if (!rowNodes.isEmpty())
        m_core.layout.append(rowNodes);
}
    m_core.row = 0;
    m_core.col = 0;
    m_core.applyProfileSettings();
    m_core.applySafetyModes();
    m_core.updateHighlight();
}

//
// Public API
//
void AACInputController::setKeyboardLayout(QGridLayout* grid,
                                           QHBoxLayout* curatedStrip,
                                           QHBoxLayout* controlRow)
{
    rebuildCoreLayout(grid, curatedStrip, controlRow);
    m_core.setState(Core::ControllerState::Idle);
}

void AACInputController::handleIntent(InputIntent intent)
{
    m_core.logIntent(intent);

    InputIntent corrected = m_core.correctIntentIfNeeded(intent);
    InputIntent predicted = m_core.predictNextIntent(); // stub; refine later

    InputIntent finalIntent = corrected; // or blend with predicted

    switch (finalIntent) {
    case InputIntent::MoveNext:
        m_core.setState(Core::ControllerState::Navigation);
        m_core.moveRight();
        break;
    case InputIntent::MovePrev:
        m_core.setState(Core::ControllerState::Navigation);
        m_core.moveLeft();
        break;
    case InputIntent::MoveUp:
        m_core.setState(Core::ControllerState::Navigation);
        m_core.moveUp();
        break;
    case InputIntent::MoveDown:
        m_core.setState(Core::ControllerState::Navigation);
        m_core.moveDown();
        break;
    case InputIntent::Activate:
        m_core.setState(Core::ControllerState::Navigation);
        m_core.activateCurrent();
        break;
    case InputIntent::Cancel:
        m_core.setState(Core::ControllerState::Idle);
        m_dwellTimer.stop();
        m_scanStepTimer.stop();
        break;
    case InputIntent::JumpToGroup:
        // wired via jumpToSemanticGroup externally
        break;
    case InputIntent::Emergency:
        m_core.setState(Core::ControllerState::Emergency);
        m_core.jumpToEmergencyItem();
        break;
    }
}

void AACInputController::handleHoverKey(AACKeyButton* btn)
{
    if (!btn)
        return;

    AACNode* node = nullptr;
    for (auto it = g_nodeToButton.constBegin(); it != g_nodeToButton.constEnd(); ++it) {
        if (it.value() == btn) {
            node = it.key();
            break;
        }
    }
    if (!node)
        node = widgetToNode(btn);

    m_dwellTimer.stop();
    m_core.handleHoverNode(node);
}

void AACInputController::handleActivation(AACKeyButton* btn)
{
    if (!btn)
        return;

    AACNode* node = nullptr;
    for (auto it = g_nodeToButton.constBegin(); it != g_nodeToButton.constEnd(); ++it) {
        if (it.value() == btn) {
            node = it.key();
            break;
        }
    }
    if (!node)
        node = widgetToNode(btn);

    m_dwellTimer.stop();
    m_core.handleActivation(node);
}

void AACInputController::setScanMode(ScanMode mode)
{
    Core::ScanMode m = Core::ScanMode::ScanOff;
    switch (mode) {
    case ScanOff:      m = Core::ScanMode::ScanOff;      break;
    case ScanLinear:   m = Core::ScanMode::ScanLinear;   break;
    case ScanRowColumn:m = Core::ScanMode::ScanRowColumn;break;
    case ScanByGroup:  m = Core::ScanMode::ScanByGroup;  break;
    case ScanLoop:     m = Core::ScanMode::ScanLoop;     break;
    case ScanBounce:   m = Core::ScanMode::ScanBounce;   break;
    }
    m_core.setScanMode(m);

    if (m == Core::ScanMode::ScanOff) {
        m_scanStepTimer.stop();
    } else {
        m_scanStepTimer.start(800);
    }
}

void AACInputController::setDwellEnabled(bool enabled)
{
    m_core.setDwellEnabled(enabled);
    if (!enabled)
        m_dwellTimer.stop();
}

void AACInputController::setDwellTime(int ms)
{
    m_core.setDwellTime(ms);
}

void AACInputController::setStepScanEnabled(bool enabled)
{
    m_core.setStepScanEnabled(enabled);
    if (!enabled)
        m_scanStepTimer.stop();
    else if (m_core.scanMode != Core::ScanMode::ScanOff)
        m_scanStepTimer.start(800);
}

void AACInputController::setSwitchControlEnabled(bool enabled)
{
    m_core.setSwitchControlEnabled(enabled);
}

//
// Timer slots
//
void AACInputController::onDwellTimeout()
{
    m_core.activateCurrent();
}

void AACInputController::onScanStepTimeout()
{
    m_core.advanceScanPosition();
    m_core.updateHighlight();
}

//
// Core methods implementation
//
void AACInputController::Core::setState(ControllerState s)
{
    state = s;
}

void AACInputController::Core::clampPosition()
{
    if (layout.isEmpty()) {
        row = 0;
        col = 0;
        return;
    }

    if (row < 0) row = 0;
    if (row >= layout.size()) row = layout.size() - 1;

    int cols = layout[row].size();
    if (cols == 0) {
        col = 0;
        return;
    }

    if (col < 0) col = 0;
    if (col >= cols) col = cols - 1;
}

void AACInputController::Core::updateHighlight()
{
    clampPosition();

    AACNode* node = nullptr;

    if (row >= 0 && row < layout.size()) {
        if (col >= 0 && col < layout[row].size()) {
            node = layout[row][col];
        }
    }

    if (onHighlightChanged)
        onHighlightChanged(node);

    if (node && onItemEntered)
        onItemEntered(node);

    if (node && semantics.contains(node)) {
        const auto& d = semantics[node];
        if (onGroupEntered)
            onGroupEntered(d.group);
    }

    if (onRowEntered)
        onRowEntered(row);
}

void AACInputController::Core::moveLeft()
{
    --col;
    clampPosition();
    updateHighlight();
}

void AACInputController::Core::moveRight()
{
    ++col;
    clampPosition();
    updateHighlight();
}

void AACInputController::Core::moveUp()
{
    --row;
    clampPosition();
    updateHighlight();
}

void AACInputController::Core::moveDown()
{
    ++row;
    clampPosition();
    updateHighlight();
}

void AACInputController::Core::activateCurrent()
{
    clampPosition();

    if (row < 0 || row >= layout.size())
        return;
    if (col < 0 || col >= layout[row].size())
        return;

    AACNode* node = layout[row][col];
    if (!node)
        return;

    if (onActivationRequested)
        onActivationRequested(node);
    if (onItemActivated)
        onItemActivated(node);

    updateSemanticHighlight(node);

    if (semantics.contains(node)) {
        const auto& d = semantics[node];
        usage.totalActivations++;
        usage.activationsByTag[d.tag]++;
        usage.activationsByGroup[d.group]++;
    }

    if (state == ControllerState::Emergency)
        state = ControllerState::Idle;

    adaptScanningAndDwell();
}

void AACInputController::Core::handleActivation(AACNode* node)
{
    if (!node)
        return;

    if (onActivationRequested)
        onActivationRequested(node);
    if (onItemActivated)
        onItemActivated(node);

    updateSemanticHighlight(node);

    if (semantics.contains(node)) {
        const auto& d = semantics[node];
        usage.totalActivations++;
        usage.activationsByTag[d.tag]++;
        usage.activationsByGroup[d.group]++;
    }

    if (state == ControllerState::Emergency)
        state = ControllerState::Idle;

    adaptScanningAndDwell();
}

void AACInputController::Core::handleHoverNode(AACNode* node)
{
    if (!node)
        return;

    for (int r = 0; r < layout.size(); ++r) {
        for (int c = 0; c < layout[r].size(); ++c) {
            if (layout[r][c] == node) {
                row = r;
                col = c;
                updateHighlight();
                return;
            }
        }
    }
}

void AACInputController::Core::updateSemanticHighlight(AACNode* node)
{
    if (!node)
        return;

    if (semantics.contains(node)) {
        const auto& d = semantics[node];
        if (onSemanticHighlightChanged)
            onSemanticHighlightChanged(d.tag);
    }
}

void AACInputController::Core::setScanMode(ScanMode mode)
{
    scanMode = mode;

    if (scanMode == ScanOff) {
        state = ControllerState::Idle;
    } else {
        state = ControllerState::Scanning;
    }
}

void AACInputController::Core::setDwellEnabled(bool enabled)
{
    dwellEnabled = enabled;
}

void AACInputController::Core::setDwellTime(int ms)
{
    dwellTimeMs = ms;
}

void AACInputController::Core::setStepScanEnabled(bool enabled)
{
    stepScanEnabled = enabled;
}

void AACInputController::Core::setSwitchControlEnabled(bool enabled)
{
    switchControlEnabled = enabled;
}

void AACInputController::Core::advanceScanPosition()
{
    if (layout.isEmpty())
        return;

    switch (scanMode) {
    case ScanOff:
        return;

    case ScanLinear:
    case ScanRowColumn:
    case ScanByGroup:
    case ScanLoop:
        ++col;
        if (row < 0 || row >= layout.size())
            row = 0;

        if (col >= layout[row].size()) {
            col = 0;
            ++row;
            if (row >= layout.size())
                row = 0;
        }
        break;

    case ScanBounce: {
        static bool forward = true;

        if (forward) {
            ++col;
            if (col >= layout[row].size()) {
                col = layout[row].size() - 1;
                forward = false;
            }
        } else {
            --col;
            if (col < 0) {
                col = 0;
                forward = true;
            }
        }
        break;
    }
    }
}

int AACInputController::Core::groupWeight(SemanticGroup g) const
{
    int base = usage.activationsByGroup.value(g, 0);

    if (g == SemanticGroup::Emotion)
        base += 2;
    else if (g == SemanticGroup::Need)
        base += 3;

    return base;
}
AACInputController::Core::SemanticGroup
AACInputController::Core::mostUsedGroup() const
{
    SemanticGroup bestGroup = SemanticGroup::Emotion;
    int bestScore = -1;

    for (auto it = usage.activationsByGroup.constBegin();
         it != usage.activationsByGroup.constEnd(); ++it) {
        int score = groupWeight(it.key());
        if (score > bestScore) {
            bestScore = score;
            bestGroup = it.key();
        }
    }

    return bestGroup;
}

AACNode* AACInputController::Core::mostUrgentItem() const
{
    AACNode* best = nullptr;
    int bestUrgency = -1;

    for (auto it = semantics.constBegin(); it != semantics.constEnd(); ++it) {
        const auto& d = it.value();
        if (d.urgency > bestUrgency) {
            bestUrgency = d.urgency;
            best = it.key();
        }
    }

    return best;
}

void AACInputController::Core::jumpToSemanticGroup(SemanticGroup group)
{
    for (auto it = semantics.constBegin(); it != semantics.constEnd(); ++it) {
        if (it.value().group == group) {
            handleHoverNode(it.key());
            return;
        }
    }
}

void AACInputController::Core::jumpToMostUsedGroup()
{
    if (usage.activationsByGroup.isEmpty())
        return;

    jumpToSemanticGroup(mostUsedGroup());
}

void AACInputController::Core::jumpToEmergencyItem()
{
    AACNode* node = mostUrgentItem();
    if (!node)
        return;

    handleHoverNode(node);
}

void AACInputController::Core::setProfile(Profile p)
{
    profile = p;
    applyProfileSettings();
}

void AACInputController::Core::applyProfileSettings()
{
    switch (profile) {
    case Profile::Default:
        dwellTimeMs = 800;
        break;
    case Profile::Blind:
        dwellTimeMs = 900;
        break;
    case Profile::LowVision:
        dwellTimeMs = 800;
        break;
    case Profile::MotorImpaired:
        dwellTimeMs = 1200;
        break;
    case Profile::CognitiveImpaired:
        dwellTimeMs = 1000;
        break;
    case Profile::SwitchUser:
        dwellTimeMs = 900;
        break;
    case Profile::EyeTrackingUser:
        dwellTimeMs = 700;
        break;
    }
}

void AACInputController::Core::adaptScanningAndDwell()
{
    if (usage.totalActivations > 0) {
        if (usage.totalActivations % 10 == 0) {
            if (dwellTimeMs > 400)
                dwellTimeMs -= 50;
        }
    }
    // --- Metadata‑aware dwell timing ---
    // Shorten dwell slightly when the most urgent semantic item is very urgent
    AACNode* urgent = mostUrgentItem();
    if (urgent && semantics.contains(urgent)) {
        const auto& d = semantics[urgent];
        if (d.urgency >= 3 && dwellTimeMs > 500)
            dwellTimeMs -= 50;
    }

    // --- Profile‑aware guardrails ---
    switch (profile) {
    case Profile::MotorImpaired:
        if (dwellTimeMs < 900) dwellTimeMs = 900;
        break;
    case Profile::Blind:
        if (dwellTimeMs < 700) dwellTimeMs = 700;
        break;
    default:
        if (dwellTimeMs < 500) dwellTimeMs = 500;
        break;
    }
}

void AACInputController::Core::applySafetyModes()
{
    // Placeholder for safety‑mode logic
}

void AACInputController::Core::handleSwitchInput(bool pressed)
{
    if (!pressed)
        return;
    // Switch mapped to Activate
}

void AACInputController::Core::handleEyeTrackingMove(int dx, int dy)
{
    eyeAccumX += dx;
    eyeAccumY += dy;

    const int threshold = 3;
    if (eyeAccumX >= threshold) {
        eyeAccumX = 0;
        moveRight();
    } else if (eyeAccumX <= -threshold) {
        eyeAccumX = 0;
        moveLeft();
    }
    if (eyeAccumY >= threshold) {
        eyeAccumY = 0;
        moveDown();
    } else if (eyeAccumY <= -threshold) {
        eyeAccumY = 0;
        moveUp();
    }
}

void AACInputController::Core::handleHeadTrackingMove(int dx, int dy)
{
    headAccumX += dx;
    headAccumY += dy;

    const int threshold = 3;
    if (headAccumX >= threshold) {
        headAccumX = 0;
        moveRight();
    } else if (headAccumX <= -threshold) {
        headAccumX = 0;
        moveLeft();
    }
    if (headAccumY >= threshold) {
        headAccumY = 0;
        moveDown();
    } else if (headAccumY <= -threshold) {
        headAccumY = 0;
        moveUp();
    }
}

void AACInputController::Core::handleGesture(const QString& gestureId)
{
    if (gestureId == "swipe_left")
        moveLeft();
    else if (gestureId == "swipe_right")
        moveRight();
    else if (gestureId == "swipe_up")
        moveUp();
    else if (gestureId == "swipe_down")
        moveDown();
    else if (gestureId == "long_press")
        activateCurrent();
    else if (gestureId == "two_finger_tap")
        jumpToEmergencyItem();
}

void AACInputController::Core::learnMotorPlanForGroup(SemanticGroup group)
{
    QVector<QPair<int,int>> path;
    for (int r = 0; r < layout.size(); ++r) {
        for (int c = 0; c < layout[r].size(); ++c) {
            AACNode* node = layout[r][c];
            if (node && semantics.contains(node) &&
                semantics[node].group == group) {
                path.append({ r, c });
            }
        }
    }
    motorPlans.insert(group, path);
}

void AACInputController::Core::executeMotorPlan(SemanticGroup group)
{
    if (!motorPlans.contains(group))
        return;

    state = ControllerState::MotorPlan;
    const auto& path = motorPlans[group];
    for (const auto& pos : path) {
        row = pos.first;
        col = pos.second;
        updateHighlight();
    }
    state = ControllerState::Idle;
}

void AACInputController::Core::startSimulation()
{
    simulationActive = true;
    state = ControllerState::Simulation;
}

void AACInputController::Core::stopSimulation()
{
    simulationActive = false;
    state = ControllerState::Idle;
}

void AACInputController::Core::replayLastSession()
{
    replayEvents(eventLog);
}

void AACInputController::Core::logIntent(InputIntent intent)
{
    if (!simulationActive)
        eventLog.append({ intent });
}

void AACInputController::Core::replayEvents(const QVector<IntentEvent>& events)
{
    for (const auto& e : events) {
        // In a full implementation, you’d route intents back through handleIntent
        Q_UNUSED(e);
    }
}

AACInputController::InputIntent
AACInputController::Core::predictNextIntent() const
{
    // Stub: return MoveNext as a neutral prediction
    return InputIntent::MoveNext;
}

AACInputController::InputIntent
AACInputController::Core::correctIntentIfNeeded(InputIntent intent) const
{
    // Stub: no correction
    return intent;
}
//
// 1. Device‑agnostic access
//
void AACInputController::Core::registerDevice(const DeviceConfig& cfg)
{
    devices.append(cfg);
}

void AACInputController::Core::updateDeviceConfig(const QString& id,
                                                  const QHash<QString, QVariant>& params)
{
    for (auto& d : devices) {
        if (d.id == id) {
            d.params = params;
            return;
        }
    }
}

AACInputController::Core::DeviceConfig
AACInputController::Core::deviceById(const QString& id) const
{
    for (const auto& d : devices) {
        if (d.id == id)
            return d;
    }
    return DeviceConfig{};
}

//
// 2. Layout compiler
//
void AACInputController::Core::compileLayoutFromGrid(const AACGrid& grid)
{
    layout = grid.rows;
    row = 0;
    col = 0;
    notifyLayoutChanged();
    updateHighlight();
}

void AACInputController::Core::compileLayoutFromStrip(const AACStrip& strip)
{
    layout.clear();
    layout.append(strip.nodes);
    row = 0;
    col = 0;
    notifyLayoutChanged();
    updateHighlight();
}

void AACInputController::Core::compileLayoutFromJson(const QString& jsonText)
{
    Q_UNUSED(jsonText);
    // Stub: parse JSON into AACGrid/AACStrip and call compileLayoutFromGrid/Strip
}

//
// 3. Semantic compiler
//
void AACInputController::Core::compileSemanticsFromJson(const QString& jsonText)
{
    Q_UNUSED(jsonText);
    // Stub: parse JSON and fill semantics map
}

void AACInputController::Core::compileSemanticsFromTags(const QHash<QString, QString>& tagToGroup)
{
    for (auto it = semantics.begin(); it != semantics.end(); ++it) {
        auto& d = it.value();
        if (tagToGroup.contains(d.tag)) {
            const QString g = tagToGroup.value(d.tag);
            if (g == "Emotion")      d.group = SemanticGroup::Emotion;
            else if (g == "Need")    d.group = SemanticGroup::Need;
            else if (g == "Social")  d.group = SemanticGroup::Social;
            else if (g == "Question")d.group = SemanticGroup::Question;
            else if (g == "Control") d.group = SemanticGroup::Control;
        }
    }
}

//
// 4. Motor‑plan authoring
//
void AACInputController::Core::defineMotorPlan(SemanticGroup group,
                                               const QVector<QPair<int,int>>& path)
{
    motorPlans.insert(group, path);
}

void AACInputController::Core::clearMotorPlan(SemanticGroup group)
{
    motorPlans.remove(group);
}

//
// 5. Semantic prediction
//
AACInputController::Core::SemanticGroup
AACInputController::Core::predictNextGroup() const
{
    return mostUsedGroup();
}

AACNode* AACInputController::Core::predictNextNode() const
{
    return mostUrgentItem();
}

//
// 6. Adaptive scanning profiles
//
void AACInputController::Core::applyScanProfile()
{
    dwellTimeMs = scanProfile.baseDwellMs;
}

void AACInputController::Core::adaptScanProfile()
{
    if (usage.totalActivations > 0 &&
        usage.totalActivations % scanProfile.adaptEveryNActs == 0) {
        if (dwellTimeMs > scanProfile.minDwellMs)
            dwellTimeMs -= 50;
    }
}

//
// 7. Evolving accessibility profiles
//
void AACInputController::Core::recordProfileUsage()
{
    ProfileHistoryEntry e;
    e.profile = profile;
    e.totalActivations = usage.totalActivations;
    e.timestamp = QDateTime::currentDateTime();
    profileHistory.append(e);
}

void AACInputController::Core::evolveProfile()
{
    if (usage.totalActivations > 100 &&
        profile == Profile::Default) {
        profile = Profile::MotorImpaired;
        applyProfileSettings();
    }
}

//
// 8. Safety layers
//
void AACInputController::Core::applySafetyLayers()
{
    // Hook for more complex safety logic
}

bool AACInputController::Core::shouldConfirmEmergency() const
{
    return safety.requireConfirmForEmergency;
}

bool AACInputController::Core::shouldConfirmDestructive() const
{
    return safety.requireConfirmForDestructive;
}

//
// 9. Session analytics
//
void AACInputController::Core::beginSession()
{
    analytics = SessionAnalytics{};
}

void AACInputController::Core::endSession()
{
    notifyAnalyticsUpdated();
}

void AACInputController::Core::recordStateSample()
{
    analytics.timeInState[state]++;
}

void AACInputController::Core::recordError()
{
    analytics.totalErrors++;
}

void AACInputController::Core::recordCorrection()
{
    analytics.totalCorrections++;
}

//
// 10. Remote control / SLP mode
//
void AACInputController::Core::setSLPRemoteActive(bool active)
{
    slpRemoteActive = active;
}

void AACInputController::Core::remoteSetHighlight(int r, int c)
{
    if (!slpRemoteActive)
        return;
    row = r;
    col = c;
    updateHighlight();
}

void AACInputController::Core::remoteActivateCurrent()
{
    if (!slpRemoteActive)
        return;
    activateCurrent();
}

void AACInputController::Core::remoteJumpToGroup(SemanticGroup group)
{
    if (!slpRemoteActive)
        return;
    jumpToSemanticGroup(group);
}

//
// 11. Multi‑user profiles
//
void AACInputController::Core::setCurrentUser(const QString& userId)
{
    currentUserId = userId;
    loadUserProfile(userId);
}

void AACInputController::Core::loadUserProfile(const QString& userId)
{
    if (!userProfiles.contains(userId))
        return;

    const auto& up = userProfiles[userId];
    profile      = up.profile;
    scanProfile  = up.scanProfile;
    safety       = up.safety;
    usage        = up.usage;
    applyProfileSettings();
    applyScanProfile();
    applySafetyLayers();
}

void AACInputController::Core::saveUserProfile(const QString& userId)
{
    UserProfileData up;
    up.userId      = userId;
    up.profile     = profile;
    up.scanProfile = scanProfile;
    up.safety      = safety;
    up.usage       = usage;
    userProfiles.insert(userId, up);
}

//
// 12. Export/import
//
QString AACInputController::Core::exportStateToJson() const
{
    // Stub: you can replace with real JSON later
    return QStringLiteral("{\"stub\":\"aac_core_state\"}");
}

void AACInputController::Core::importStateFromJson(const QString& jsonText)
{
    Q_UNUSED(jsonText);
    // Stub: parse and restore state
}

//
// 13. Plug‑in architecture
//
void AACInputController::Core::registerPlugin(const Plugin& plugin)
{
    plugins.insert(plugin.id, plugin);
    if (plugin.onLoad)
        plugin.onLoad(*this);
}

void AACInputController::Core::unloadPlugin(const QString& id)
{
    if (!plugins.contains(id))
        return;
    Plugin p = plugins[id];
    if (p.onUnload)
        p.onUnload(*this);
    plugins.remove(id);
}

//
// 14. Cross‑platform rendering hooks
//
void AACInputController::Core::notifyLayoutChanged()
{
    if (onLayoutChanged)
        onLayoutChanged(layout);
}

void AACInputController::Core::notifyUsageUpdated()
{
    if (onUsageUpdated)
        onUsageUpdated(usage);
}

void AACInputController::Core::notifyAnalyticsUpdated()
{
    if (onAnalyticsUpdated)
        onAnalyticsUpdated(analytics);
}

//
// 15. Testing harness
//
void AACInputController::Core::addTestCase(const TestCase& tc)
{
    testCases.append(tc);
}

void AACInputController::Core::runTestCase(const QString& id)
{
    for (const auto& tc : testCases) {
        if (tc.id == id) {
            for (auto intent : tc.sequence) {
                logIntent(intent);
                // In a full harness, you’d route through handleIntent
            }
            return;
        }
    }
}

void AACInputController::Core::runAllTestCases()
{
    for (const auto& tc : testCases)
        runTestCase(tc.id);
}
