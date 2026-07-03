package org.aacframework;

import android.content.Context;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.provider.Settings;
import android.view.ViewConfiguration;
import android.view.accessibility.AccessibilityManager;
import android.accessibilityservice.AccessibilityServiceInfo;

import java.util.List;

public final class HapticHelper {

    private static AccessibilityManager.AccessibilityStateChangeListener listener;

    private HapticHelper() {}

    // ------------------------------------------------------------
    // 1. Register listener for instant TalkBack updates
    // ------------------------------------------------------------
    public static void registerAccessibilityListener(Context ctx) {
        AccessibilityManager am =
                (AccessibilityManager) ctx.getSystemService(Context.ACCESSIBILITY_SERVICE);

        if (am == null)
            return;

        listener = enabled -> {
            boolean tb = isTalkBackEnabled(ctx);
            nativeOnTalkBackChanged(tb);
        };

        am.addAccessibilityStateChangeListener(listener);
    }

    // ------------------------------------------------------------
    // 2. Live TalkBack detection
    // ------------------------------------------------------------
    public static boolean isTalkBackEnabled(Context ctx) {
        AccessibilityManager am =
                (AccessibilityManager) ctx.getSystemService(Context.ACCESSIBILITY_SERVICE);
        if (am == null)
            return false;

        List<AccessibilityServiceInfo> enabled =
                am.getEnabledAccessibilityServiceList(
                        AccessibilityServiceInfo.FEEDBACK_SPOKEN);

        return enabled != null && !enabled.isEmpty();
    }

    // ------------------------------------------------------------
    // 3. System haptic enabled?
    // ------------------------------------------------------------
    public static boolean isHapticFeedbackEnabled(Context ctx) {
        try {
            int enabled = Settings.System.getInt(
                    ctx.getContentResolver(),
                    Settings.System.HAPTIC_FEEDBACK_ENABLED,
                    1
            );
            return enabled == 1;
        } catch (Exception e) {
            return true;
        }
    }

    // ------------------------------------------------------------
    // 4. Touch & hold delay
    // ------------------------------------------------------------
    public static int getTouchAndHoldDelayMs(Context ctx) {
        return ViewConfiguration.get(ctx).getLongPressTimeout();
    }

    // ------------------------------------------------------------
    // 5. Large targets preference (font scale)
    // ------------------------------------------------------------
    public static boolean prefersLargeTargets(Context ctx) {
        try {
            float scale = ctx.getResources().getConfiguration().fontScale;
            return scale >= 1.2f;
        } catch (Exception e) {
            return false;
        }
    }

    // ------------------------------------------------------------
    // 6. Vibrator capability
    // ------------------------------------------------------------
    public static boolean hasVibrator(Context ctx) {
        Vibrator v = (Vibrator) ctx.getSystemService(Context.VIBRATOR_SERVICE);
        if (v == null)
            return false;

        if (Build.VERSION.SDK_INT >= 26)
            return v.hasVibrator();

        return true;
    }

    // ------------------------------------------------------------
    // 7. Perform haptic + live TalkBack update
    // ------------------------------------------------------------
    public static boolean performHaptic(Context ctx, int strength) {
        // Live TalkBack update on every haptic call
        boolean tb = isTalkBackEnabled(ctx);
        nativeOnTalkBackChanged(tb);

        if (!isHapticFeedbackEnabled(ctx))
            return false;

        Vibrator v = (Vibrator) ctx.getSystemService(Context.VIBRATOR_SERVICE);
        if (v == null)
            return false;

        if (Build.VERSION.SDK_INT >= 26) {
            int amplitude;
            int duration;

            switch (strength) {
                case 1:
                    duration = 30;
                    amplitude = 64;
                    break;
                case 2:
                    duration = 40;
                    amplitude = 128;
                    break;
                default:
                    duration = 50;
                    amplitude = 196;
                    break;
            }

            VibrationEffect effect =
                    VibrationEffect.createOneShot(duration, amplitude);
            v.vibrate(effect);
        } else {
            long duration;
            switch (strength) {
                case 1: duration = 30; break;
                case 2: duration = 40; break;
                default: duration = 50; break;
            }
            v.vibrate(duration);
        }

        return true;
    }

    // ------------------------------------------------------------
    // 8. JNI callback
    // ------------------------------------------------------------
    public static native void nativeOnTalkBackChanged(boolean enabled);
}
