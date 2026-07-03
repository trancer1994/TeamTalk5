#pragma once
#include "AACScreenBase.h"

class AACVocabularyScreen : public AACScreenBase
{
    Q_OBJECT
public:
    explicit AACVocabularyScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

signals:
    void vocabularyChosen(const QString& vocabId);
};
