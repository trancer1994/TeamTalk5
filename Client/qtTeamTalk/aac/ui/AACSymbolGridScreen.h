#pragma once

#include "AACScreenBase.h"
#include <QVector>

class QGridLayout;
class AACAccessibilityManager;
class AACSymbolButton;
struct AACVocabItem;

class AACSymbolGridScreen : public AACScreenBase
{
    Q_OBJECT

public:
    explicit AACSymbolGridScreen(AACAccessibilityManager* aac,
                                 QWidget* parent = nullptr);

signals:
    void symbolActivated(const QString& label);
    void keyboardRequested();

private slots:
    void onHighContrastChanged(bool enabled);
    void onSymbolClicked(const QString& label);

private:
    void rebuildGrid();
    void publishScanningLayout();

    // AACScreenAdapter overrides via AACScreenBase
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;

private:
    AACAccessibilityManager* m_aac = nullptr;
    QGridLayout* m_layout = nullptr;
    QVector<AACSymbolButton*> m_buttons;
};
