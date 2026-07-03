#include <Xinput.h>
#pragma comment(lib, "Xinput9_1_0.lib")

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Gaming.Input.h>
using namespace winrt;
using namespace Windows::Gaming::Input;

namespace HapticBackend_Windows {

// ---------------------------
// XInput rumble (gamepads)
// ---------------------------
void perform(int strength)
{
    WORD level = (strength == 1 ? 15000 :
                  strength == 2 ? 30000 :
                                  45000);

    XINPUT_VIBRATION vib;
    ZeroMemory(&vib, sizeof(vib));
    vib.wLeftMotorSpeed  = level;
    vib.wRightMotorSpeed = level;

    XInputSetState(0, &vib);

    QTimer::singleShot(40, [](){
        XINPUT_VIBRATION stop = {0, 0};
        XInputSetState(0, &stop);
    });
}

// ---------------------------
// WinRT haptics (Surface trackpads, actuators)
// ---------------------------
bool performWinRT(int strength)
{
    init_apartment();

    auto controllers = RawGameController::RawGameControllers();
    if (controllers.Size() == 0)
        return false;

    for (auto const& c : controllers) {
        auto simple = c.SimpleHapticsController();
        if (!simple)
            continue;

        auto feedbacks = simple.SupportedFeedback();
        if (feedbacks.Size() == 0)
            continue;

        auto fb = feedbacks.GetAt(0);

        double amp = 0.5;
        if (strength == 1) amp = 0.3;
        if (strength == 2) amp = 0.6;
        if (strength == 3) amp = 1.0;

        simple.SendHapticFeedback(fb, amp, std::chrono::milliseconds(30));
        return true;
    }

    return false;
}

} // namespace HapticBackend_Windows
