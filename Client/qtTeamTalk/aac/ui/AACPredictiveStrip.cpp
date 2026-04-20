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
    auto suggestions = engine->Predict(prefix, 5);

    updateButtons(suggestions);
}

void PredictiveStrip::updateButtons(const std::vector<std::string>& suggestions)
{
    QStringList newList;
    for (const auto& s : suggestions)
        newList << QString::fromStdString(s);

    // Map existing buttons by text
    QHash<QString, AACButton*> oldMap;
    for (AACButton* btn : m_buttons)
        oldMap.insert(btn->text(), btn);

    // Clear layout (but not widgets yet)
    while (m_layout->count() > 0) {
        QLayoutItem* item = m_layout->takeAt(0);
        delete item;
    }

    QList<AACButton*> newButtons;

    for (const QString& word : newList) {
        AACButton* btn = nullptr;

        if (oldMap.contains(word)) {
            btn = oldMap.value(word);
            oldMap.remove(word);
        } else {
            btn = new AACButton(m_mgr, this);
            btn->setText(word);
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

            // Confidence styling
            if (m_mgr && m_mgr->predictionEngine()) {
                float conf = m_mgr->predictionEngine()->confidenceFor(word.toStdString());
                if (conf < 0.0f) conf = 0.0f;
                if (conf > 1.0f) conf = 1.0f;
                applyConfidenceStyling(btn, conf);
            }

            connect(btn, &QPushButton::clicked,
                    this, [this, word]() {
                        emit suggestionChosen(word);
                        if (m_mgr && m_mgr->predictionEngine()) {
                            const QString trimmed = word.trimmed();
                            if (!trimmed.isEmpty())
                                m_mgr->predictionEngine()->learnUtterance(trimmed);
                        }
                    });
        }

        m_layout->addWidget(btn);
        newButtons.append(btn);
    }

    // Delete leftover buttons (not reused)
    for (AACButton* leftover : oldMap.values()) {
        leftover->deleteLater();
    }

    m_buttons = newButtons;
    m_layout->addStretch();
}

void PredictiveStrip::applyAACProperties(AACButton* btn)
{
    if (!btn || !m_mgr)
        return;

    btn->setProperty("aacHighContrast", m_mgr->highContrastEnabled());
    btn->setProperty("aacDwellEnabled", m_mgr->dwellEnabled());

    applyAdaptiveSizing(btn);
}

void PredictiveStrip::applyConfidenceStyling(AACButton* btn, float conf)
{
    int shade = 232 - static_cast<int>(conf * 24);
    int focusShade = shade - 8;

    btn->setStyleSheet(QString(
        "AACButton { "
        "  background-color: rgb(%1, %1, %1); "
        "  border-radius: 12px; "
        "  border: 1px solid #c8c8c8; "
        "} "
        "AACButton:focus { "
        "  background-color: rgb(%2, %2, %2); "
        "}"
    ).arg(shade).arg(focusShade));
}

void PredictiveStrip::applyAdaptiveSizing(AACButton* btn)
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
