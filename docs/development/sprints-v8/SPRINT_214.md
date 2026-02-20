# Sprint 214 — Configuration Migration Engine

**Sprint Number:** 214
**Version:** v10.1.0
**Status:** ✅ Complete

## Objective
Implement a configuration migration engine that handles schema versioning, key renaming, value transformation, and validation when upgrading between DarkThumbs versions.

## Files Changed
- `Engine/Utils/ConfigMigrationEngine.h` — Header with ConfigVersion, MigrationAction enums, MigrationRule struct, ConfigMigrationEngine class
- `Engine/Utils/ConfigMigrationEngine.cpp` — Full implementation with migration rules, backup/restore, validation
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests added

## Tests Added (5)
1. `TestConfigMigration_VersionNames` — Version name resolution
2. `TestConfigMigration_ActionNames` — Action name resolution
3. `TestConfigMigration_BasicMigration` — End-to-end migration with defaults
4. `TestConfigMigration_RenameRule` — Key rename during migration
5. `TestConfigMigration_Validation` — Schema validation (required keys)

## Key Features
- ConfigVersion enum: V7.0 through V10.0 (7 versions)
- MigrationAction enum: Keep, Rename, Transform, Remove, AddDefault
- Rule-based migration with per-action application
- Schema-driven default value injection for new version keys
- Backup/restore support for safe migration rollback
- Configuration validation against required schema entries
