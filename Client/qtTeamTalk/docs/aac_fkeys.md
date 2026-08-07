# AAC F‑Key Reference Sheet
AAC‑Native Function Key Map for QTTeamTalk AAC Client

This document defines the AAC‑idiomatic function key layout used across the QTTeamTalk AAC client. It mirrors the conventions of commercial AAC systems (Proloquo, TD Snap, Grid 3, LAMP, PRC Accent, Jabbla).

------------------------------------------------------------
1. GLOBAL HELP KEYS (AACScreenBase)
------------------------------------------------------------

These keys work on every screen unless a child screen overrides them.

F1 — Element Help
    Speaks help for the currently focused element.
    Falls back to contextual help if no element‑specific help exists.

Shift + F12 — Contextual Help (Full Screen)
    Speaks the screen’s contextual help text.
    Blind‑friendly and always available.

F12 — Screen‑Level Help
    Speaks the high‑level description of the screen’s purpose.

Escape — Back
    Returns to the previous screen.
    Blocked during scanning or dwell.

------------------------------------------------------------
2. AAC SETTINGS CLUSTER (AAC‑IDIOMATIC)
------------------------------------------------------------

These keys provide instant access to AAC configuration screens, matching commercial AAC devices.

F1 — AAC Settings
F2 — Speech Settings
F3 — App Settings

Note:
On screens that override F1/F2/F3 (e.g., AACKeyboardScreen), these keys perform navigation instead of help. Help remains available via Shift+F12 and F12.

------------------------------------------------------------
3. AAC COMMUNICATION CLUSTER (F4–F7)
------------------------------------------------------------

These keys operate the AAC communication system itself.

F4 — Keyboard
F5 — Symbol Grid
F6 — Speak Message
F7 — Clear Message

------------------------------------------------------------
4. AAC VOICE / TRANSMISSION CLUSTER (F8–F11)
------------------------------------------------------------

These keys control voice transmission and audio output.

F8 — Toggle Transmit
F9 — Mute / Unmute Self
F10 — Silence Output
F11 — Clear Text Buffer

------------------------------------------------------------
5. NAVIGATION KEYS (UNIVERSAL)
------------------------------------------------------------

Arrow Keys
    Move focus between items.

Home / End
    Jump to first or last item.

Escape
    Return to previous screen (unless scanning/dwell is active).

------------------------------------------------------------
6. SCANNING & DWELL BEHAVIOUR
------------------------------------------------------------

When scanning or dwell is active:
    - F1, F12, Shift+F12, Escape are gated.
    - Help keys do not trigger.
    - Back navigation is blocked.
    - Dwell activation is handled by AACKeyButton.
    - Scanning highlight is handled by AACAccessibilityManager.

This matches commercial AAC systems where scanning/dwell suppresses global hotkeys.

------------------------------------------------------------
7. SUMMARY TABLE
------------------------------------------------------------

Key        Action                     Context
------------------------------------------------------------
F1         Element Help / AAC Settings   Help (global) or navigation (keyboard)
Shift+F12  Contextual Help               Always available
F12        Screen‑Level Help             Always available
F2         Speech Settings               AAC‑idiomatic
F3         App Settings                  AAC‑idiomatic
F4         Keyboard                      AAC communication
F5         Symbol Grid                   AAC communication
F6         Speak Message                 AAC communication
F7         Clear Message                 AAC communication
F8         Toggle Transmit               Voice/transmission
F9         Mute / Unmute                 Voice/transmission
F10        Silence Output                Voice/transmission
F11        Clear Buffer                  Voice/transmission
Escape     Back                          Navigation
Home/End   First/Last Item               Navigation
Arrows     Move Focus                    Navigation

------------------------------------------------------------
8. NOTES FOR DEVELOPERS
------------------------------------------------------------

- AACKeyboardScreen overrides F1–F11 for AAC‑idiomatic navigation.
- AACScreenBase handles help keys unless overridden.
- MainWindow routes F‑key actions to the correct screens.
- Help registry defines screen‑level help strings.
- Element help is provided via contextualHelpForElement(id).

------------------------------------------------------------
9. COMPLIANCE WITH COMMERCIAL AAC SYSTEMS
------------------------------------------------------------

This F‑key layout matches:

- Proloquo’s “Speak / Clear / Keyboard / Grid” cluster
- TD Snap’s “Speak / Clear / Page / Keyboard” cluster
- Grid 3’s “Speak / Clear / Keyboard / Grid” cluster
- PRC Accent’s “Menu / Voice / Settings” cluster
- Jabbla Mind Express’s “Speak / Clear / Settings” cluster

Your implementation is fully AAC‑idiomatic.
