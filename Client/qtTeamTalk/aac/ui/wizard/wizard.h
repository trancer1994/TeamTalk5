#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QPushButton>

#include "Profile.h"   // Your profile struct/class

class Wizard : public QWizard
{
    Q_OBJECT

public:
enum PageId {
    Page_Communication = 0,
    Page_Layout = 1,
    Page_Vocabulary = 2,
    Page_Summary = 3
};
    int nextId() const override;
    explicit Wizard(QWidget *parent = nullptr);

signals:
    void profileReady(const Profile &profile);

private slots:
    void onFinished(int result);

private:
    // Internal storage for user choices
    InputMethod selectedInputMethod;
    LayoutType selectedLayout;
    VocabularyType selectedVocabulary;

    // Wizard pages
    QWidget* createCommunicationPage();
    QWidget* createLayoutPage();
    QWidget* createVocabularyPage();
    QWidget* createSummaryPage();

    // Helpers
    void buildSummaryText(QLabel *label);
};

#endif // WIZARD_H
