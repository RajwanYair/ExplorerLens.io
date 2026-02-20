# Sprint 241 — CommandLineInterface

**Date:** 2026-06-15
**Component:** `Engine/Utils/CommandLineInterface.h`, `Engine/Utils/CommandLineInterface.cpp`
**Theme:** CLI Argument Parser for CBXManager

## Summary
Implemented a CLI argument parser supporting flag, string, integer, file path, and enum argument types. Features include short/long name mapping, required argument validation, enum allowed-value enforcement, help generation, and descriptive error messages.

## Key Types
- `ArgType` — Flag, String, Integer, FilePath, Enum
- `ParseStatus` — Success, UnknownArgument, MissingValue, MissingRequired, InvalidValue, HelpRequested
- `ArgDefinition` — long/short name, description, type, required flag, default value, allowed values
- `ParsedArg` — name, value, type, isPresent

## Tests Added (5)
- `TestCLI_ParseFlags` — flag argument parsed correctly
- `TestCLI_ParseString` — string value extraction
- `TestCLI_MissingRequired` — missing required arg returns MissingRequired
- `TestCLI_HelpRequested` — --help returns HelpRequested
- `TestCLI_ArgTypeNames` — type and status name resolution

## Files Modified
- `Engine/CMakeLists.txt` — registered header + source
- `Engine/Tests/EngineTests.cpp` — 5 tests + RUN_TEST calls
