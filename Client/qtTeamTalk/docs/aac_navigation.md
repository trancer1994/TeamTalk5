# AAC Navigation System Documentation
AAC‑Native Navigation Architecture for QTTeamTalk AAC Client

This document describes the AAC navigation system implemented in the QTTeamTalk AAC client. It explains how users move between screens, how focus navigation works, how AAC‑idiomatic function keys provide rapid access to communication tools, and how scanning/dwell systems interact with navigation.

------------------------------------------------------------
1. OVERVIEW
------------------------------------------------------------

AAC navigation must be:
    - predictable
    - low‑cognitive‑load
    - consistent across screens
    - accessible to blind users
    - compatible with scanning and dwell
    - AAC‑idiomatic (matching commercial AAC systems)

The QTTeamTalk AAC client implements a unified navigation model across all screens.

------------------------------------------------------------
2. FUNCTION KEY NAVIGATION
------------------------------------------------------------

AAC systems use function keys for rapid navigation. The QTTeamTalk AAC client follows this convention.

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

These keys are handled by AACKeyboardScreen and AACMainScreen. Other screens inherit default behaviour from AACScreenBase.

------------------------------------------------------------
3. SCREEN NAVIGATION
------------------------------------------------------------

Escape — Back
    Returns to the previous screen.
    Blocked during scanning or dwell.

Home / End
    Jump to the first or last item in lists or grids.

Arrow Keys
    Move focus between items.

Tab / Shift+Tab
    Move focus between interactive elements.

------------------------------------------------------------
4. FOCUS NAVIGATION
------------------------------------------------------------

Focus navigation is handled by Qt’s focus system and AACAccessibilityManager.

Rules:
    - Arrow keys move focus between grid or list items.
    - Tab moves focus between buttons and controls.
    - Focus is always visible via highlight or scanning indicator.
    - Focus movement is suppressed during dwell activation.

------------------------------------------------------------
5. SCREEN TRANSITIONS
------------------------------------------------------------

Screen transitions are handled by MainWindow. Each AAC screen is a QWidget that replaces the central widget.

Transitions:
    - Keyboard ↔ Symbol Grid
    - Keyboard ↔ Settings
    - Symbol Grid ↔ Settings
    - Main Screen ↔ Channel List
    - Channel List ↔ InChannel

Transitions must:
    - preserve focus
    - preserve accessibility state
    - preserve scanning/dwell state
    - reset contextual help

------------------------------------------------------------
6. SCANNING NAVIGATION
------------------------------------------------------------

Scanning is controlled by AACAccessibilityManager.

Behaviour:
    - Scanning highlights one element at a time.
    - Dwell or switch activation selects the highlighted element.
    - Arrow keys are disabled during scanning.
    - Escape, F1, F12, Shift+F12 are gated.

Scanning must:
    - follow a predictable order
    - be interruptible
    - be restartable
    - be consistent across screens

------------------------------------------------------------
7. DWELL NAVIGATION
------------------------------------------------------------

Dwell activation is handled by AACKeyButton.

Behaviour:
    - Hovering over a button triggers a dwell timer.
    - When the timer completes, the button activates.
    - Dwell suppresses keyboard navigation.
    - Dwell suppresses help keys.

Dwell must:
    - provide visual feedback
    - be cancellable
    - be consistent across screens

------------------------------------------------------------
8. SUMMARY
------------------------------------------------------------

The AAC navigation system provides:
    - AAC‑idiomatic function key navigation
    - predictable focus movement
    - consistent screen transitions
    - blind‑friendly behaviour
    - scanning/dwell compatibility
    - low‑cognitive‑load operation

This matches commercial AAC systems and ensures reliable, accessible navigation.
