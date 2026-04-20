#include "AACSymbolGridScreen.h"
#include "AACSymbolButton.h"
#include "AACAccessibilityManager.h"
#include "AACFramework.h"

#include <QGridLayout>
#include <QLayoutItem>

AACSymbolGridScreen::AACSymbolGridScreen(AACAccessibilityManager* aac,
                                         QWidget* parent)
    : AACScreenBase(parent)
    , m_aac(aac)
{
    setScreenTitle("Symbols");

    m_layout = new QGridLayout(this);
    m_layout->setSpacing(12);
    m_layout->setContentsMargins(12, 12, 12, 12);

    rebuildGrid();
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

// ------------------------------------------------------------
// Symbol activation
// ------------------------------------------------------------

void AACSymbolGridScreen::onSymbolClicked(const QString& label)
{
    emit symbolActivated(label);

    if (!m_aac)
        return;

    // Insert text into the AAC input controller
    if (auto* ic = m_aac->inputController()) {
        QMetaObject::invokeMethod(ic, "insertText",
                                  Q_ARG(QString, label + " "));
    }

    // Speak the symbol
    if (auto* se = m_aac->speechEngine())
        se->speak(label);
}
