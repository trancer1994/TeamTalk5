#import <UIKit/UIKit.h>

namespace HapticBackend_iOS {

void perform(int strength)
{
    @autoreleasepool {

        if (strength == 1) {
            static UIImpactFeedbackGenerator* genLight =
                [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
            [genLight prepare];
            [genLight impactOccurred];

        } else if (strength == 2) {
            static UIImpactFeedbackGenerator* genMedium =
                [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium];
            [genMedium prepare];
            [genMedium impactOccurred];

        } else {
            static UINotificationFeedbackGenerator* genError =
                [[UINotificationFeedbackGenerator alloc] init];
            [genError prepare];
            [genError notificationOccurred:UINotificationFeedbackTypeError];
        }
    }
}

}
