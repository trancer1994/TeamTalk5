#pragma once

#include <QString>

class AACElementMetadata
{
public:
    QString helpText;          // Spoken when focused
    QString semanticTag;       // For semantic highlighting
    QString role;              // AAC role (button, label, status, etc.)
    QString description;       // Longer AAC description
    bool suppressInFatigue = false;
};
