#pragma once
#include <QVector>
#include "AACProfile.h"

namespace AAC {
namespace Core {

inline const QVector<AACProfileConfig>& profileConfigs()
{
    static const QVector<AACProfileConfig> configs = {
        { AACProfile::CoreVocabulary,
          "core", "arasaac_core", "core_5x7", "full" },

        { AACProfile::ConcreteCommunicator,
          "concrete", "photos_concrete", "concrete_3x3", "off" },

        { AACProfile::HighContrast,
          "highContrast", "arasaac_high_contrast", "hc_3x4", "full" },

        { AACProfile::TextOnly,
          "textOnly", "none", "keyboard", "text" },

        { AACProfile::MotorAccess,
          "motor", "arasaac_core", "scanning_1xn", "limited" },

        { AACProfile::SensoryEmotion,
          "sensory", "sensory_emotion", "sensory_2x2", "off" }
    };
    return configs;
}

inline AACProfileConfig profileConfig(AACProfile p)
{
    for (const auto& cfg : profileConfigs())
        if (cfg.profile == p)
            return cfg;
    return profileConfigs().first();
}

} // namespace Core
} // namespace AAC
