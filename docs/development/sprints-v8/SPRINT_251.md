# Sprint 251: cbxArchive.h Split — FormatTypes.h

**Status:** ✅ Complete  
**Version:** 10.6.0  
**Date:** 2026-02-20  
**Phase:** Phase 1 — Architecture & Quality Foundation

## Objective
Create `FormatTypeLookup` as a `std::unordered_map` lookup table replacing the monolithic `GetCBXType()` if-else chain. Maps 80+ extensions to `FormatType` values.

## Deliverables
- `Engine/Core/FormatTypes.h` — FormatTypeLookup singleton with 80+ extension mappings
- Archives (26 extensions), Modern Images (6), Professional (15), Netpbm (6), v8.4+ formats (18), Scientific (5), Film (2), Text/Data (6)
- `LookupStats` for validation reporting
- 5 unit tests
