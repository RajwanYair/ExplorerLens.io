#pragma once
//==============================================================================
// ExplorerLens Engine — Unified Dark Mode Module
//
// Consolidates: DarkModeEngine, DarkModeRendererV2, DarkModeControls
//
// Section 1: DarkModeEngine — Theme detection, color roles (platform-independent)
// Section 2: DarkModeRendererV2 — Owner-draw rendering (Win32 GDI)
// Section 3: DarkModeControls — Per-control dark mode drawing (Win32 GDI)
//==============================================================================

// Platform-independent theme engine (always available)
#include "DarkModeEngine.h"

// Win32 GDI-based rendering (Windows only)
#ifdef _WIN32
#include "DarkModeRendererV2.h"
#include "DarkModeControls.h"
#endif
