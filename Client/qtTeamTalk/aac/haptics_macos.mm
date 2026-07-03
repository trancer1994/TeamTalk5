#import <AppKit/AppKit.h>

namespace HapticBackend_macOS {

bool perform(int strength)
{
    @autoreleasepool {
        NSHapticFeedbackManager* mgr = [NSHapticFeedbackManager defaultPerformer];
        if (!mgr)
            return false;

        NSHapticFeedbackPattern pattern;
        NSHapticFeedbackPerformanceLevel level;

        switch (strength) {
            case 1:
                pattern = NSHapticFeedbackPatternGeneric;
                level   = NSHapticFeedbackPerformanceLevelLow;
                break;

            case 2:
                pattern = NSHapticFeedbackPatternGeneric;
                level   = NSHapticFeedbackPerformanceLevelDefault;
                break;

            default:
                pattern = NSHapticFeedbackPatternLevelChange;
                level   = NSHapticFeedbackPerformanceLevelHigh;
                break;
        }

        [mgr performFeedbackPattern:pattern performanceLevel:level];
        return true;
    }
}

}
