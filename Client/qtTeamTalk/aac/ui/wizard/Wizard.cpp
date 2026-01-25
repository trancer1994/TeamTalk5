#include "Wizard.h"
#include <QTextEdit>
#include <QDebug>

Wizard::Wizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle("Setup Wizard");

    // Page 1: Communication method
    setPage(0, createCommunicationPage());

    // Page 2: Layout
    setPage(1, createLayoutPage());

    // Page 3: Vocabulary
    setPage(2, createVocabularyPage());

    // Page 4: Summary
    setPage(3, createSummaryPage());

    connect(this, &QWizard::finished,
            this, &Wizard::onFinished);
}

//
// PAGE 1 — "How do you communicate?"
//
QWidget* Wizard::createCommunicationPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *title = new QLabel("How do you communicate?");
    title->setWordWrap(true);
    layout->addWidget(title);

    QButtonGroup *group = new QButtonGroup(page);

    QRadioButton *touch = new QRadioButton("I touch the screen\n"
                                           "I tap things directly with my finger or a stylus.");
    QRadioButton *gaze = new QRadioButton("I look at things\n"
                                          "I use my eyes to choose things on the screen.");
    QRadioButton *scan = new QRadioButton("I wait for things to be highlighted\n"
                                          "The app moves through options and I press a switch.");
    QRadioButton *dwell = new QRadioButton("I hover over things\n"
                                           "I hold my finger, pointer, or gaze until it selects.");

    group->addButton(touch, 0);
    group->addButton(gaze, 1);
    group->addButton(scan, 2);
    group->addButton(dwell, 3);

    layout->addWidget(touch);
    layout->addWidget(gaze);
    layout->addWidget(scan);
    layout->addWidget(dwell);

    // Default
    touch->setChecked(true);

    // Store selection when leaving page
    connect(group, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int id){
        switch (id) {
        case 0: selectedInputMethod = InputMethod::Touch; break;
        case 1: selectedInputMethod = InputMethod::Gaze; break;
        case 2: selectedInputMethod = InputMethod::Scanning; break;
        case 3: selectedInputMethod = InputMethod::Dwell; break;
        }
    });

    return page;
}

//
// PAGE 2 — Layout
//
QWidget* Wizard::createLayoutPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *title = new QLabel("How do you build messages?");
    title->setWordWrap(true);
    layout->addWidget(title);

    QButtonGroup *group = new QButtonGroup(page);

    QRadioButton *grid = new QRadioButton("Symbol Grid");
    QRadioButton *keyboard = new QRadioButton("Keyboard");
    QRadioButton *both = new QRadioButton("Both");

    group->addButton(grid, 0);
    group->addButton(keyboard, 1);
    group->addButton(both, 2);

    layout->addWidget(grid);
    layout->addWidget(keyboard);
    layout->addWidget(both);

    grid->setChecked(true);

    connect(group, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int id){
        switch (id) {
        case 0: selectedLayout = LayoutType::Grid; break;
        case 1: selectedLayout = LayoutType::Keyboard; break;
        case 2: selectedLayout = LayoutType::Hybrid; break;
        }
    });

    return page;
}

//
// PAGE 3 — Vocabulary
//
QWidget* Wizard::createVocabularyPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *title = new QLabel("Choose your vocabulary");
    title->setWordWrap(true);
    layout->addWidget(title);

    QButtonGroup *group = new QButtonGroup(page);

    QRadioButton *core = new QRadioButton("Core Vocabulary");
    QRadioButton *expanded = new QRadioButton("Expanded Vocabulary");
    QRadioButton *custom = new QRadioButton("Custom Vocabulary");

    group->addButton(core, 0);
    group->addButton(expanded, 1);
    group->addButton(custom, 2);

    layout->addWidget(core);
    layout->addWidget(expanded);
    layout->addWidget(custom);

    core->setChecked(true);

    connect(group, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int id){
        switch (id) {
        case 0: selectedVocabulary = VocabularyType::Core; break;
        case 1: selectedVocabulary = VocabularyType::Expanded; break;
        case 2: selectedVocabulary = VocabularyType::Custom; break;
        }
    });

    return page;
}

//
// PAGE 4 — Summary
//
QWidget* Wizard::createSummaryPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *title = new QLabel("Summary");
    layout->addWidget(title);

    QLabel *summary = new QLabel;
    summary->setWordWrap(true);
    layout->addWidget(summary);

    // Update summary when page is shown
    connect(this, &QWizard::currentIdChanged,
            this, [this, summary](int id){
        if (id == 3)
            buildSummaryText(summary);
    });

    return page;
}

//
// Build summary text
//
void Wizard::buildSummaryText(QLabel *label)
{
    QString inputStr;
    switch (selectedInputMethod) {
    case InputMethod::Touch: inputStr = "Touch"; break;
    case InputMethod::Gaze: inputStr = "Gaze"; break;
    case InputMethod::Scanning: inputStr = "Scanning"; break;
    case InputMethod::Dwell: inputStr = "Dwell"; break;
    }

    QString layoutStr;
    switch (selectedLayout) {
    case LayoutType::Grid: layoutStr = "Symbol Grid"; break;
    case LayoutType::Keyboard: layoutStr = "Keyboard"; break;
    case LayoutType::Hybrid: layoutStr = "Both"; break;
    }

    QString vocabStr;
    switch (selectedVocabulary) {
    case VocabularyType::Core: vocabStr = "Core"; break;
    case VocabularyType::Expanded: vocabStr = "Expanded"; break;
    case VocabularyType::Custom: vocabStr = "Custom"; break;
    }

    label->setText(
        "Input Method: " + inputStr + "\n"
        "Layout: " + layoutStr + "\n"
        "Vocabulary: " + vocabStr
    );
}

//
// FINISH — Emit the profile
//
void Wizard::onFinished(int result)
{
    if (result != QDialog::Accepted)
        return;

    Profile p;
    p.inputMethod = selectedInputMethod;
    p.layout = selectedLayout;
    p.vocabulary = selectedVocabulary;

    // Mode flags derived from input method
    p.modeFlags.fromInputMethod(selectedInputMethod);

    emit profileReady(p);
}
