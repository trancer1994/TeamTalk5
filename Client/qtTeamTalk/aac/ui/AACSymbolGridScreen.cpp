#include "AACSymbolGridScreen.h"
#include "AACSymbolButton.h"
#include "AACFramework.h"

#include <QGridLayout>
#include <QLayoutItem>

AACSymbolGridScreen::AACSymbolGridScreen(AACAccessibilityManager* aac,
                                         QWidget* parent)
    : AACScreenBase(parent)
    , m_aac(aac)
{
    setScreenTitle("Symbols");

auto* root = new QVBoxLayout(this);

// Top row with Keyboard toggle
auto* topRow = new QHBoxLayout();
auto* keyboardBtn = new AACKeyButton(tr("Keyboard"), m_aac, this);
topRow->addWidget(keyboardBtn);
topRow->addStretch(1);
root->addLayout(topRow);

connect(keyboardBtn, &AACKeyButton::keyActivated,
        this, [this]() { emit keyboardRequested(); });

// Grid layout
m_layout = new QGridLayout();
    m_layout->setSpacing(12);
    m_layout->setContentsMargins(12, 12, 12, 12);
root->addLayout(m_layout);

if (m_aac) {
    connect(m_aac, &AACAccessibilityManager::highContrastChanged,
            this, &AACSymbolGridScreen::onHighContrastChanged);
}

    rebuildGrid();
publishScanningLayout();
}
QString AACSymbolGridScreen::contextualHelp() const
{
    return tr("AACSymbolGrid. "
               "Press F4 for Keyboard. "
               "Press F6 to speak your message. "
               "Press Escape to go back.");
}

// ------------------------------------------------------------
// AACScreenAdapter overrides
// ------------------------------------------------------------

QList<QWidget*> AACSymbolGridScreen::interactiveWidgets() const
{
    QList<QWidget*> widgets;
    for (auto* b : m_buttons)
        widgets.append(b);
    return widgets;
}

QList<QWidget*> AACSymbolGridScreen::primaryWidgets() const
{
    // For symbol grids, all buttons are primary AAC targets.
    QList<QWidget*> widgets;
    for (auto* b : m_buttons)
        widgets.append(b);
    return widgets;
}

QLayout* AACSymbolGridScreen::rootLayout() const
{
    return m_layout;
}

// ------------------------------------------------------------
// Grid rebuild
// ------------------------------------------------------------

void AACSymbolGridScreen::rebuildGrid()
{
    // Clear old widgets
    QLayoutItem* item = nullptr;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }
    m_buttons.clear();

    if (!m_aac)
        return;

    const QString category = m_aac->activeCategory();
    const QVector<AACVocabItem> words = m_aac->words(category);

    const int columns = 4;
    int row = 0;
    int col = 0;

    for (const AACVocabItem& itemData : words) {
        AACSymbolButton* btn = new AACSymbolButton(
            itemData.label,
            itemData.iconPath,
            m_aac,
            this
        );

        connect(btn, &AACSymbolButton::symbolActivated,
                this, &AACSymbolGridScreen::onSymbolClicked);

        m_layout->addWidget(btn, row, col);
        m_buttons.append(btn);

        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
}

void AACSymbolGridScreen::publishScanningLayout()
{
    if (!m_aac)
        return;

    QVector<QVector<QWidget*>> layout;

    QVector<QWidget*> row;
    for (auto* b : m_buttons)
        row.append(b);

    if (!row.isEmpty())
        layout.append(row);

    m_aac->setKeyboardScanningLayout(layout);
}

// ------------------------------------------------------------
// Symbol activation
// ------------------------------------------------------------

void AACSymbolGridScreen::onHighContrastChanged(bool enabled)
{
    for (auto* b : m_buttons) {
        if (!b) continue;
        b->setProperty("aacHighContrast", enabled);
        b->style()->unpolish(b);
        b->style()->polish(b);
        b->update();
    }
}
void AACSymbolGridScreen::onSymbolClicked(const QString& label)
{
    emit symbolActivated(label);

    if (m_aac && m_aac->predictionEngine()) {
        const QString trimmed = label.trimmed();
        if (!trimmed.isEmpty())
            m_aac->predictionEngine()->learnUtterance(trimmed.toStdString());
    }
}
