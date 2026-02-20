# Sprint 279: Auto-Update Engine

**Status:** ✅ Complete  
**Version:** v11.2  

## Objective
Silent background update checking, download, and staged rollout with rollback.

## Deliverables
- `Engine/Utils/AutoUpdateEngine.h` — 4 channels, 6 check results, 7 download states
- Version parsing (A.B.C), rollback support, SHA256 verification
- AutoUpdateConfig with channel, interval, auto-download/install flags
- 5 unit tests: channel names, check results, download states, version parsing, counts
