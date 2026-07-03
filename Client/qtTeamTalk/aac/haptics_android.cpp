#include <QAndroidJniObject>
#include <QtAndroid>

namespace HapticBackend_Android {

bool perform(int strength)
{
    QAndroidJniObject ctx = QtAndroid::androidContext();
    if (!ctx.isValid())
        return false;

    // 1. Vibrator capability
    jboolean hasVib = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/aacframework/HapticHelper",
        "hasVibrator",
        "(Landroid/content/Context;)Z",
        ctx.object()
    );

    if (!hasVib)
        return false;   // fallback needed

    // 2. TalkBack detection
    jboolean talkback = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/aacframework/HapticHelper",
        "isTalkBackEnabled",
        "(Landroid/content/Context;)Z",
        ctx.object()
    );

    // 3. Deliver vibration using the Java helper
    jboolean delivered = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/aacframework/HapticHelper",
        "performHaptic",
        "(Landroid/content/Context;I)Z",
        ctx.object(),
        strength
    );

    // If TalkBack is active, we still return true because
    // the Java helper already softened the amplitude.
    return delivered;
}

} // namespace
