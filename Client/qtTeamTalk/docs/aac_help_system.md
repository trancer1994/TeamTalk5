# AAC Help System Documentation
AAC‑Native Help Architecture for QTTeamTalk AAC Client

This document describes the AAC help system implemented in the QTTeamTalk AAC client. It explains how help keys behave, how AACScreenBase processes help requests, how child screens override help behaviour, and how help remains accessible for blind AAC users.

------------------------------------------------------------
1. OVERVIEW
------------------------------------------------------------

The AAC help system provides spoken, context‑aware assistance to AAC users. It is designed to be fully accessible, blind‑friendly, and consistent across all screens.

The help system has three layers:
    - Element‑level help
    - Contextual help (screen‑level context)
    - Screen‑level help (high‑level description)

These layers are triggered by specific function keys.

------------------------------------------------------------
2. HELP KEYS
------------------------------------------------------------

The following keys activate the AAC help system:

F1 — Element Help
    Speaks help for the currently focused element.
    If no element‑specific help exists, falls back to contextual help.

Shift + F12 — Contextual Help
    Speaks the screen’s contextual help text.
    Always available, even when F1 is overridden by child screens.

F12 — Screen‑Level Help
    Speaks the high‑level description of the screen’s purpose.

Escape — Back
    Returns to the previous screen.
    Blocked during scanning or dwell.

------------------------------------------------------------
3. HELP PROCESSING FLOW
------------------------------------------------------------

Qt delivers key events from the focused widget upward through the parent hierarchy. The help system relies on this propagation model.

Event flow:
    1. Focused child widget receives the key event.
    2. If the child handles the key, the event stops.
    3. If not, the event propagates to its parent.
    4. Eventually AACScreenBase::keyPressEvent receives the event.

This allows child screens to override help keys while preserving global help behaviour.

------------------------------------------------------------
4. AACSCREENBASE HELP HANDLING
------------------------------------------------------------

AACScreenBase implements the following help functions:

screenLevelHelp()
    Returns a high‑level description of the screen.

contextualHelp()
    Returns contextual help for the screen.
    Used when no element‑specific help is available.

contextualHelpForElement(id)
    Returns help text for a specific element.
    Child screens may override this to provide detailed help.

AACScreenBase::keyPressEvent handles:
    - F1 (element help)
    - Shift+F12 (contextual help)
    - F12 (screen‑level help)
    - Escape (back navigation)

------------------------------------------------------------
5. CHILD SCREEN OVERRIDES
------------------------------------------------------------

Some screens override help keys to provide AAC‑idiomatic navigation.

Example: AACKeyboardScreen overrides F1–F11 for navigation:
    F1 — AAC Settings
    F2 — Speech Settings
    F3 — App Settings
    F4 — Keyboard
    F5 — Symbol Grid
    F6 — Speak Message
    F7 — Clear Message
    F8 — Toggle Transmit
    F9 — Mute / Unmute
    F10 — Silence Output
    F11 — Clear Buffer

Because AACKeyboardScreen handles F1, AACScreenBase does not receive F1 on that screen. Help remains available via Shift+F12 and F12.

Other screens (Settings, Speech Settings, App Settings, Channel List, InChannel, Connect) do not override F1, so AACScreenBase handles help normally.

------------------------------------------------------------
6. BLIND‑FRIENDLY HELP ACCESS
------------------------------------------------------------

Blind AAC users must be able to access help even when navigation keys override F1. The help system ensures this by providing:

Shift + F12 — Contextual Help
    Always available.
    Never overridden by child screens.

F12 — Screen‑Level Help
    Always available.
    Never overridden by child screens.

This guarantees that help is accessible on every screen.

------------------------------------------------------------
7. SCANNING AND DWELL BEHAVIOUR
------------------------------------------------------------

When scanning or dwell is active:
    - Help keys are gated.
    - F1, F12, Shift+F12, Escape do not trigger.
    - Back navigation is blocked.
    - Dwell activation is handled by AACKeyButton.
    - Scanning highlight is handled by AACAccessibilityManager.

This matches commercial AAC systems where scanning/dwell suppresses global hotkeys.

------------------------------------------------------------
8. IMPLEMENTATION NOTES
------------------------------------------------------------

- AACScreenBase must be the parent class for all AAC screens.
- Child screens may override keyPressEvent to implement AAC‑idiomatic navigation.
- Help strings should be defined in each screen’s implementation of:
      screenLevelHelp()
      contextualHelp()
      contextualHelpForElement(id)
- MainWindow may provide a help registry for screen‑level help.
- Help must be spoken using the AAC speech engine.

------------------------------------------------------------
9. SUMMARY
------------------------------------------------------------

The AAC help system provides:
    - Element‑level help via F1
    - Contextual help via Shift+F12
    - Screen‑level help via F12
    - Blind‑friendly access to help
    - Consistent behaviour across screens
    - AAC‑idiomatic overrides in AACKeyboardScreen
    - Scanning/dwell gating for accessibility

This architecture matches commercial AAC systems and ensures that help is always accessible, predictable, and low‑cognitive‑load.
