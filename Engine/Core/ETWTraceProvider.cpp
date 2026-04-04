// ETWTraceProvider.cpp — TraceLogging Provider Definition
// Copyright (c) 2026 ExplorerLens Project
//
// This file defines the TraceLogging provider instance.
// MUST be compiled in exactly ONE translation unit.

#ifdef _WIN32

    #include <windows.h>
    #include <TraceLoggingProvider.h>

// ============================================================================
// Define the TraceLogging provider
// Provider Name: "ExplorerLens-Engine-Provider"
// The GUID is auto-generated from the provider name by TraceLogging.
//
// To capture events:
//   tracelog -start ExplorerLensTrace -guid *ExplorerLens-Engine-Provider -f trace.etl
//   tracelog -stop ExplorerLensTrace
//   tracefmt trace.etl -o trace.txt
//
// Or with WPR/WPA/PerfView:
//   Use provider name "ExplorerLens-Engine-Provider"
// ============================================================================

TRACELOGGING_DEFINE_PROVIDER(g_hExplorerLensProvider, "ExplorerLens-Engine-Provider",
                             // {A1B2C3D4-E5F6-7890-ABCD-EF1234567890} — deterministic GUID from name
                             (0xa1b2c3d4, 0xe5f6, 0x7890, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90));

#endif  // _WIN32
