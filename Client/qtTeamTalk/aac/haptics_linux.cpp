#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>

namespace HapticBackend_Linux {

bool perform(int strength)
{
    // Try scanning event devices
    for (int i = 0; i < 32; ++i) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);

        int fd = open(path, O_RDWR | O_NONBLOCK);
        if (fd < 0)
            continue;

        // Check if device supports FF_RUMBLE
        unsigned long features[4] = {0};
        if (ioctl(fd, EVIOCGBIT(EV_FF, sizeof(features)), features) < 0) {
            close(fd);
            continue;
        }

        if (!(features[FF_RUMBLE / 8] & (1 << (FF_RUMBLE % 8)))) {
            close(fd);
            continue;
        }

        // Build rumble effect
        ff_effect effect {};
        effect.type = FF_RUMBLE;

        if (strength == 1) {
            effect.u.rumble.strong_magnitude = 0x2000;
            effect.u.rumble.weak_magnitude   = 0x2000;
        } else if (strength == 2) {
            effect.u.rumble.strong_magnitude = 0x5000;
            effect.u.rumble.weak_magnitude   = 0x5000;
        } else {
            effect.u.rumble.strong_magnitude = 0x8000;
            effect.u.rumble.weak_magnitude   = 0x8000;
        }

        effect.replay.length = 40; // ms
        effect.replay.delay  = 0;

        // Upload effect
        if (ioctl(fd, EVIOCSFF, &effect) < 0) {
            close(fd);
            continue;
        }

        // Play effect
        input_event play {};
        play.type = EV_FF;
        play.code = effect.id;
        play.value = 1;

        write(fd, &play, sizeof(play));

        // Auto-stop after 40ms
        usleep(40000);

        play.value = 0;
        write(fd, &play, sizeof(play));

        close(fd);
        return true;
    }

    return false; // No haptic device found
}

}
