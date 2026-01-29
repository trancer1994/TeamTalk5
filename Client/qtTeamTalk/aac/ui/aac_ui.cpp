#include "aacui.h"
#include "aac_grid_model.h"

#include <QGridLayout>
#include <QPushButton>
#include <QIcon>
#include <QSize>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace AAC {

AACUI::AACUI(QWidget* parent)
    : QWidget(parent)
{
    // Nothing here yet — model is applied later
}

void AACUI::setModel(AACGridModel* model)
{
    m_model = model;

    auto* mainLayout = new QVBoxLayout(this);

    //
    // CATEGORY BAR
    //
    auto* catLayout = new QHBoxLayout();
    auto* coreBtn = new QPushButton("Core", this);
    auto* peopleBtn = new QPushButton("People", this); // placeholder example

    catLayout->addWidget(coreBtn);
    catLayout->addWidget(peopleBtn);

    connect(coreBtn, &QPushButton::clicked, this, [this]() {
        emit categorySelected("core");
    });
    connect(peopleBtn, &QPushButton::clicked, this, [this]() {
        emit categorySelected("people");
    });

    mainLayout->addLayout(catLayout);

    //
    // GRID LAYOUT
    //
    auto* grid = new QGridLayout();
    const auto& entries = m_model->entries();

    int row = 0;
    int col = 0;

    for (const auto& entry : entries) {
        auto* btn = new QPushButton(entry.label, this);
        btn->setIcon(QIcon(entry.iconPath));
        btn->setIconSize(QSize(64, 64));

        grid->addWidget(btn, row, col);

        connect(btn, &QPushButton::clicked, this, [this, label = entry.label]() {
            emit symbolActivated(label);
        });

        if (++col == 8) {
            col = 0;
            ++row;
        }
    }

    mainLayout->addLayout(grid);
    setLayout(mainLayout);
}

} // namespace AAC
