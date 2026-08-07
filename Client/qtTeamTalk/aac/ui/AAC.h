#pragma once

#include <QString>

class QWidget;
class AACAccessibilityManager;

class AACAccessibilityManager; // forward if needed

namespace AAC {

inline AACAccessibilityManager* manager(); // you already have a way to get this

inline void setElementHelp(QWidget* w, const QString& help)
{
    auto* reg = manager()->registry();
    AACElementMetadata md = reg->metadata(w);
    md.helpText = help;
    reg->setMetadata(w, md);
}

inline void setSemanticTag(QWidget* w, const QString& tag)
{
    auto* reg = manager()->registry();
    AACElementMetadata md = reg->metadata(w);
    md.semanticTag = tag;
    reg->setMetadata(w, md);
}

inline void setRole(QWidget* w, const QString& role)
{
    auto* reg = manager()->registry();
    AACElementMetadata md = reg->metadata(w);
    md.role = role;
    reg->setMetadata(w, md);
}

inline void setDescription(QWidget* w, const QString& desc)
{
    auto* reg = manager()->registry();
    AACElementMetadata md = reg->metadata(w);
    md.description = desc;
    reg->setMetadata(w, md);
}

inline void suppressInFatigue(QWidget* w, bool on = true)
{
    auto* reg = manager()->registry();
    AACElementMetadata md = reg->metadata(w);
    md.suppressInFatigue = on;
    reg->setMetadata(w, md);
}

} // namespace AAC
