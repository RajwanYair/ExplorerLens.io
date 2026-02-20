# Sprint 283: Database Preview (SQLite)

**Status:** ✅ Complete  
**Version:** v12.0  

## Objective
SQLite/database file thumbnail with schema and data grid rendering.

## Deliverables
- `Engine/Decoders/DatabasePreviewDecoder.h` — 4 DB engines, 7 column types, 4 preview styles
- SQLite magic byte detection ("SQLite format 3\0")
- Schema view with table/column listing, ER diagram option
- 5 unit tests: engines, column types, styles, SQLite magic, counts
