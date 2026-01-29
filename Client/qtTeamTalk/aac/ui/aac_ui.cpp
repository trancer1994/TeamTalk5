#include "aacui.h"
#include "aac_grid_model.h"
#include <QGridLayout>
#include <QPushButton>

namespace AAC {

AACUI::AACUI(QWidget* parent)
    : QWidget(parent)
{
}

void AACUI::setModel(AACGridModel* model)
{
    m_model = model;

    auto* layout = new QGridLayout(this);

    const auto& entries = m_model->entries();

    int row = 0;
    int col = 0;

    for (const auto& entry : entries) {
        QPushButton* btn = new QPushButton(entry.label);
        btn->setIcon(QIcon(entry.iconPath));
        btn->setIconSize(QSize(64, 64));

        layout->addWidget(btn, row, col);

        col++;
        if (col == 8) {   // 48-core = 6 rows × 8 columns
            col = 0;
            row++;
        }
    }

    setLayout(layout);
}

} // namespace AAC
