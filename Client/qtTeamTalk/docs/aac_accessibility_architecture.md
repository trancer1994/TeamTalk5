# AAC Accessibility Architecture Documentation
Accessibility Framework for QTTeamTalk AAC Client

This document describes the accessibility architecture of the QTTeamTalk AAC client. It explains how scanning, dwell, speech output, earcons, focus management, and help systems integrate to provide a fully accessible AAC experience.

------------------------------------------------------------
1. OVERVIEW
------------------------------------------------------------

The AAC accessibility architecture consists of:
    - AACAccessibilityManager
    - AACKeyButton
    - Speech Engine
    - Earcon System
    - Help System
    - Focus Management
    - Scanning and Dwell

These components work together to provide a consistent, accessible user experience.

------------------------------------------------------------
2. AACACCESSIBILITYMANAGER
------------------------------------------------------------

AACAccessibilityManager coordinates:
    - scanning
    - dwell suppression
    - accessibility state
    - highlight rendering
    - gating of help keys
    - gating of navigation keys

Responsibilities:
    - Start/stop scanning
    - Advance scanning highlight
    - Trigger dwell activation
    - Block Escape, F1, F12, Shift+F12 during scanning/dwell
    - Provide accessibility state to screens

------------------------------------------------------------
3. AACKEYBUTTON
------------------------------------------------------------

AACKeyButton is the core interactive element for AAC users.

Features:
    - dwell activation
    - highlight rendering
    - speech feedback
    - earcon feedback
    - scanning compatibility

Dwell behaviour:
    - Hover starts dwell timer
    - Timer completion activates button
    - Movement cancels dwell

------------------------------------------------------------
4. SPEECH ENGINE
------------------------------------------------------------

The speech engine provides:
    - element help
    - contextual help
    - screen‑level help
    - message speaking
    - earcon playback

Speech output must:
    - be interruptible
    - be non‑blocking
    - support queued messages
    - support priority messages (help, warnings)

------------------------------------------------------------
5. EARCON SYSTEM
------------------------------------------------------------

Earcons provide non‑speech audio feedback.

Used for:
    - focus changes
    - scanning highlight movement
    - dwell activation
    - button activation
    - warnings
    - errors

Earcons must:
    - be short
    - be distinct
    - not overlap speech output

------------------------------------------------------------
6. HELP SYSTEM INTEGRATION
------------------------------------------------------------

Help keys interact with accessibility:

F1 — Element Help
Shift+F12 — Contextual Help
F12 — Screen‑Level Help

During scanning/dwell:
    - help keys are gated
    - help does not trigger
    - scanning/dwell takes priority

------------------------------------------------------------
7. FOCUS MANAGEMENT
------------------------------------------------------------

Focus management ensures:
    - focus is always visible
    - scanning highlight matches focus
    - dwell activation targets the focused element
    - help targets the focused element

------------------------------------------------------------
8. SCANNING
------------------------------------------------------------

Scanning cycles through interactive elements.

Rules:
    - one element highlighted at a time
    - dwell or switch activates the element
    - keyboard navigation is disabled
    - help keys are disabled
    - scanning is consistent across screens

------------------------------------------------------------
9. DWELL
------------------------------------------------------------

Dwell activation:
    - starts when pointer hovers over an element
    - completes after dwell timeout
    - activates the element
    - cancels if pointer moves

Dwell suppresses:
    - keyboard navigation
    - help keys
    - scanning transitions

------------------------------------------------------------
10. SUMMARY
------------------------------------------------------------

The AAC accessibility architecture provides:
    - scanning
    - dwell
    - speech output
    - earcons
    - help integration
    - focus management
    - accessibility gating

This architecture matches commercial AAC systems and ensures full accessibility for blind, motor‑impaired, and switch‑based AAC users.
