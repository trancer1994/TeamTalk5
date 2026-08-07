#include "AACPredictiveStrip.h"

#include "AACFramework.h"
#include "AACPredictionEngine.h"
#include "AACTextBar.h"

#include <QHBoxLayout>
#include <QSizePolicy>
#include <QHash>
#include <QStringList>

PredictiveStrip::PredictiveStrip(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
m_semanticPreviewLabel = new QLabel(this);
m_semanticPreviewLabel->setVisible(false);
m_semanticPreviewLabel->setStyleSheet(
    "font-size: 20px; padding: 4px; color: #444;"
);

m_layout->addWidget(m_semanticPreviewLabel);

    m_debounceTimer.setSingleShot(true);
    connect(&m_debounceTimer, &QTimer::timeout,
            this, &PredictiveStrip::debouncedUpdate);
}

void PredictiveStrip::setManager(AACAccessibilityManager* mgr)
{
    m_mgr = mgr;

    if (!m_mgr)
        return;

    // React to AAC global settings
    connect(m_mgr, &AACAccessibilityManager::largeTargetsChanged,
            this, [this](bool){ debouncedUpdate(); });

    connect(m_mgr, &AACAccessibilityManager::highContrastChanged,
            this, [this](bool){ debouncedUpdate(); });

    connect(m_mgr, &AACAccessibilityManager::dwellChanged,
            this, [this](bool){ debouncedUpdate(); });
}

void PredictiveStrip::setTextBar(AACTextBar* tb)
{
    m_textBar = tb;
}
std::vector<std::string> PredictiveStrip::currentSuggestionList() const
{
    std::vector<std::string> out;
    out.reserve(m_buttons.size());

    for (AACKeyButton* btn : m_buttons) {
        if (!btn)
            continue;
        out.push_back(btn->text().toStdString());
    }

    return out;
}

void PredictiveStrip::setContext(const QString& text)
{
    if (!m_mgr || !m_mgr->predictionEnabled())
        return;

    m_pendingContext = text;
    m_debounceTimer.start(40);
}

void PredictiveStrip::onCharacterTyped(const QString& /*ch*/)
{
    if (!m_textBar)
        return;

    setContext(m_textBar->text());
}

void PredictiveStrip::debouncedUpdate()
{
    if (!m_mgr || !m_mgr->predictionEnabled())
        return;

    AACPredictionEngine* engine = m_mgr->predictionEngine();
    if (!engine)
        return;

    const std::string prefix = m_pendingContext.toStdString();
auto suggestions = engine->Predict(prefix, m_maxSuggestions);
    updateButtons(suggestions);
}

void PredictiveStrip::updateButtons(const std::vector<std::string>& suggestions)
{
    // #2 — Handle empty suggestion lists FIRST
    if (suggestions.empty()) {
        while (m_layout->count() > 0)
            delete m_layout->takeAt(0);

        m_buttons.clear();
        m_lastSuggestionList.clear();
        m_layout->addStretch();
        return;
    }

    // Build newList ONCE
    QStringList newList;
    for (const auto& s : suggestions)
        newList << QString::fromStdString(s);

    // #3 — Avoid rebuilding identical strips
    if (newList == m_lastSuggestionList)
        return;

    m_lastSuggestionList = newList;

    // Map existing buttons by text
    QHash<QString, AACKeyButton*> oldMap;
    for (AACKeyButton* btn : m_buttons)
        oldMap.insert(btn->text(), btn);

    // Clear layout (but not widgets yet)
    while (m_layout->count() > 0) {
        QLayoutItem* item = m_layout->takeAt(0);
        delete item;
    }

    QList<AACKeyButton*> newButtons;

    for (const QString& word : newList) {
        AACKeyButton* btn = nullptr;

        if (oldMap.contains(word)) {
            btn = oldMap.value(word);
            oldMap.remove(word);
        } else {
            btn = new AACKeyButton(m_mgr, this);
    btn->setObjectName("prediction_" + word);
btn->setText(word);
QString elided = btn->fontMetrics().elidedText(word, Qt::ElideRight, 120);
btn->setText(elided);
m_mgr->input()->registerInteractive(btn, true);
int totalWidth = this->width();
int btnWidth = totalWidth / m_maxSuggestions;
btn->setMinimumWidth(btnWidth);
btn->setMaximumWidth(btnWidth);
            btn->setDeepWell(true);

            applyAACProperties(btn);

            // Dwell override based on word length
            if (m_mgr) {
                int base = m_mgr->dwellConfig().dwellDurationMs;
                int len  = word.length();
                int overrideMs = base;

                if (len <= 2)      overrideMs = base * 0.65;
                else if (len <= 4) overrideMs = base * 0.75;
                else if (len <= 7) overrideMs = base * 0.85;

                btn->setDwellOverrideMs(overrideMs);
            }

            // Confidence + semantic styling
            float conf = 0.0f;
            float sem  = 0.0f;
            if (m_mgr && m_mgr->predictionEngine()) {
                AACPredictionEngine* engine = m_mgr->predictionEngine();
                conf = std::clamp(engine->confidenceFor(word.toStdString()), 0.0f, 1.0f);
                sem  = std::max(0.0f, engine->semanticWeightForToken(word.toStdString()));
            }

            applyConfidenceStyling(btn, conf, sem);

            connect(btn, &QPushButton::clicked,
                    this, [this, word]() {
            //
            // #6 — VISUAL FLASH (instant)
            //
            QString originalStyle = btn->styleSheet();
            btn->setStyleSheet(originalStyle +
                "AACKeyButton { background-color: #d0ffd0; }");

            QTimer::singleShot(120, this, [btn, originalStyle]() {
                btn->setStyleSheet(originalStyle);
            });


            //
            // #6b — HAPTIC PULSE (if supported)
            //
#ifdef Q_OS_ANDROID
            QAndroidJniObject vibrator = QAndroidJniObject::callStaticObjectMethod(
                "android/os/Vibrator", "from",
                "(Landroid/content/Context;)Landroid/os/Vibrator;",
                QtAndroid::androidContext().object()
            );
            if (vibrator.isValid()) {
                vibrator.callMethod<void>("vibrate", "(J)V", 30LL); // 30ms pulse
            }
#endif

#ifdef Q_OS_IOS
            UIImpactFeedbackGenerator* generator =
                [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
            [generator impactOccurred];
#endif


            //
            // #6c — SYNTHETIC CLICK SOUND (no WAV file)
            //
            if (m_mgr && m_mgr->feedbackEngine()) {
                m_mgr->feedbackEngine()->playClickTone();  // <-- You add this helper
            }

                        emit suggestionChosen(word);
                        if (m_mgr && m_mgr->predictionEngine()) {
                            const QString trimmed = word.trimmed();
                            if (!trimmed.isEmpty())
                                m_mgr->predictionEngine()->learnUtterance(trimmed);
                        }
                    });
        }

AAC::setElementHelp(btn, tr("Prediction: %1").arg(word));
AAC::setRole(btn, "prediction");

        m_layout->addWidget(btn);
        newButtons.append(btn);
    }

    // Delete leftover buttons (not reused)
    for (AACKeyButton* leftover : oldMap.values())
        leftover->deleteLater();

    m_buttons = newButtons;
    m_layout->addStretch();
}

QList<QWidget*> PredictiveStrip::interactiveWidgets() const
{
    QList<QWidget*> out;
    for (auto* b : m_buttons)
        out.append(b);
    return out;
}

QList<QWidget*> PredictiveStrip::primaryWidgets() const
{
    return interactiveWidgets();
}
void PredictiveStrip::clear()
{
    while (m_layout->count() > 0)
        delete m_layout->takeAt(0);

    m_buttons.clear();
    m_lastSuggestionList.clear();
    m_layout->addStretch();
}
void PredictiveStrip::setMaxSuggestions(int n)
{
    m_maxSuggestions = std::max(1, std::min(n, 12)); // clamp 1–12
}
void PredictiveStrip::applyAACProperties(AACKeyButton* btn)
{
    if (!btn || !m_mgr)
        return;

    btn->setProperty("aacHighContrast", m_mgr->highContrastEnabled());
    btn->setProperty("aacDwellEnabled", m_mgr->dwellEnabled());

    applyAdaptiveSizing(btn);
}

void PredictiveStrip::applyConfidenceStyling(AACKeyButton* btn, float conf, float semanticWeight)
{
    // Base shade from confidence
    int shade = 232 - static_cast<int>(conf * 24);

    // Darken further if semantically boosted
    if (semanticWeight > 0.0f) {
        float factor = std::min(1.0f, semanticWeight / 3.0f);
        shade -= static_cast<int>(factor * 20);
    }

    if (shade < 180) shade = 180;
    int focusShade = shade - 8;

    btn->setStyleSheet(QString(
        "AACKeyButton { "
        "  background-color: rgb(%1, %1, %1); "
        "  border-radius: 12px; "
        "  border: 1px solid #c8c8c8; "
        "} "
"AACKeyButton:hover { "
"  border: 2px solid #888; "
"} "
"AACKeyButton:focus { "
"  border: 2px solid #555; "
"  background-color: rgb(%2, %2, %2); "
"} "
    ).arg(shade).arg(focusShade));
}

void PredictiveStrip::applyAdaptiveSizing(AACKeyButton* btn)
{
    if (!m_mgr)
        return;

    const AACModeFlags modes = m_mgr->modes();
    if (!modes.largeTargets)
        return;

    btn->setMinimumSize(AAC_MIN_TARGET, AAC_MIN_TARGET);

    QFont f = btn->font();
    f.setPointSizeF(f.pointSizeF() * AAC_FONT_SCALE);
    btn->setFont(f);
}
void PredictiveStrip::setSemanticContext(const QString& tag)
{
    m_currentSemanticTag = tag;

    if (tag.isEmpty()) {
        m_semanticPreviewLabel->clear();
        m_semanticPreviewLabel->setVisible(false);
        return;
    }

    // Show a static preview icon or text
    QString sym = symbolForSemanticTag(tag);
    if (!sym.isEmpty())
        m_semanticPreviewLabel->setText(sym);
    else
        m_semanticPreviewLabel->setText(tag);

    m_semanticPreviewLabel->setVisible(true);
}

void PredictiveStrip::clearSemanticContext()
{
    m_currentSemanticTag.clear();
    m_semanticPreviewLabel->clear();
    m_semanticPreviewLabel->setVisible(false);
}
