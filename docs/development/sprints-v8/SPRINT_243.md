# Sprint 243 — NotificationEngine

**Date:** 2026-06-15
**Component:** `Engine/Utils/NotificationEngine.h`, `Engine/Utils/NotificationEngine.cpp`
**Theme:** Toast Notifications for Batch/Update Events

## Summary
Implemented a notification engine managing toast-style notifications with 7 event types, 4 priority levels, and 5 states. Features include automatic duration scaling by priority, max-notification cap with overflow handling, type/state filtering, dismiss/click tracking, and expiration cleanup.

## Key Types
- `NotifyType` — BatchComplete, UpdateAvailable, DecoderError, CacheCleared, PluginLoaded, LicenseExpiring, SystemWarning
- `NotifyPriority` — Low, Normal, High (2x duration), Critical (3x duration)
- `NotifyState` — Pending, Displayed, Dismissed, Expired, Clicked
- `Notification` — id, type, priority, state, title, message, actionUrl, timestamps, duration

## Tests Added (5)
- `TestNotify_Send` — send creates notification with valid id
- `TestNotify_Dismiss` — dismiss changes state to Dismissed
- `TestNotify_ByType` — filter by type returns correct count
- `TestNotify_TypeNames` — all 7 type names resolve
- `TestNotify_PriorityNames` — priority and state name resolution

## Files Modified
- `Engine/CMakeLists.txt` — registered header + source
- `Engine/Tests/EngineTests.cpp` — 5 tests + RUN_TEST calls
