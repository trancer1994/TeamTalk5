#pragma once

#include <QString>

namespace AAC {
namespace Core {

enum class AACProfile {
    CoreVocabulary,
    ConcreteCommunicator,
    HighContrast,
    TextOnly,
    MotorAccess,
    SensoryEmotion
};

struct AACProfileConfig {
    AACProfile profile;
    QString id;
    QString symbolPack;
    QString gridId;
    QString prediction;
};

} // namespace Core
} // namespace AAC
