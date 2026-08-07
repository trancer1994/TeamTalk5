# AAC Prediction Engine Documentation
Semantic Prediction Architecture for QTTeamTalk AAC Client

This document describes the AAC prediction engine used in the QTTeamTalk AAC client. It explains how semantic context, next‑word prediction, and AAC‑specific language models integrate with the keyboard and symbol grid.

------------------------------------------------------------
1. OVERVIEW
------------------------------------------------------------

The AAC prediction engine provides several functions:
- next‑word prediction
- semantic context prediction
- AAC‑specific vocabulary support
- symbol grid prediction
- keyboard prediction

Prediction is designed to reduce cognitive load, accelerate communication, and adapt to user context.

------------------------------------------------------------
2. SEMANTIC CONTEXT MODEL
------------------------------------------------------------

The prediction engine uses semantic context derived from multiple sources:
- previous words
- the current sentence
- the active AAC screen
- the active channel
- user preferences

Context identifiers include:
- keyboard context
- symbol grid context
- channel context
- settings context

------------------------------------------------------------
3. NEXT WORD PREDICTION
------------------------------------------------------------

Next‑word prediction uses AAC‑specific vocabulary and semantic context. It relies on:
- frequency models
- recency models
- AAC‑specific language patterns

Predictions appear above the keyboard and in the symbol grid prediction row.

------------------------------------------------------------
4. SYMBOL GRID PREDICTION
------------------------------------------------------------

Symbol grid prediction provides:
- predicted symbols
- predicted categories
- predicted phrases

Predictions are based on semantic context, symbol frequency, and user history.

------------------------------------------------------------
5. KEYBOARD PREDICTION
------------------------------------------------------------

Keyboard prediction provides:
- next‑word suggestions
- next‑phrase suggestions
- corrections

Predictions update after each keypress, dwell activation, or symbol selection.

------------------------------------------------------------
6. MESSAGE BUFFER INTEGRATION
------------------------------------------------------------

The prediction engine reads from the AAC message buffer. Prediction updates occur when:
- the buffer changes
- the buffer is cleared
- a sentence boundary is detected

------------------------------------------------------------
7. ACCESSIBILITY INTEGRATION
------------------------------------------------------------

Prediction integrates with scanning, dwell, and speech output.

During scanning:
- prediction candidates are highlighted
- dwell activation selects a prediction
- speech output speaks the selected prediction

------------------------------------------------------------
8. PERFORMANCE REQUIREMENTS
------------------------------------------------------------

Prediction must remain responsive. It must:
- update quickly
- avoid blocking the UI
- avoid blocking speech output
- remain responsive during scanning and dwell

------------------------------------------------------------
9. SUMMARY
------------------------------------------------------------

The AAC prediction engine provides:
- semantic context prediction
- next‑word prediction
- symbol grid prediction
- keyboard prediction
- accessibility integration

This architecture matches commercial AAC systems and supports fast, low‑effort communication.
